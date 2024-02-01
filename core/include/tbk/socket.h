#ifndef __TBK_UDP_SOCKET_H__
#define __TBK_UDP_SOCKET_H__
#include <string>
#include <array>
#include <iostream>
#include <random>
#include <chrono>
#include <cstdlib>
#include <boost/asio.hpp>
#include <fmt/format.h>
#include "tbk/type.h"
#include "tbk/log.h"
#include "tbk/posix.h"
#include "tbk/utils/singleton.h"

namespace tbk{
using __io = Singleton<boost::asio::io_context>;
namespace udp{
using __callback_type = tbk::type::socket_callback_type;
using __callback_ep_type = tbk::type::socket_ep_callback_type;
using endpoint = boost::asio::ip::udp::endpoint;
using address = boost::asio::ip::address;
using address_v4 = boost::asio::ip::address_v4;
class socket{
public:
    socket():_socket(*__io::_(),boost::asio::ip::udp::v4()){}
    ~socket(){
        close();
    }
    void close(){
        _socket.close();
    }
    bool is_running(){
        return _is_running;
    };
    bool bind(const boost::asio::ip::udp::endpoint& _listen_ep,const __callback_type& f = {}){
        return _bind(_listen_ep,f,{});
    }
    bool bind(const boost::asio::ip::udp::endpoint& _listen_ep,const __callback_ep_type& f = {}){
        return _bind(_listen_ep,{},f);
    }
    bool bind(const std::string& ip,const int& port,const __callback_type& f = {}){
        boost::asio::ip::udp::endpoint _listen_ep(boost::asio::ip::address::from_string(ip),port);
        return bind(_listen_ep,f);
    }
    bool bind_any(const std::string& ip,const __callback_type& f={}){
        auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937 rng{static_cast<std::mt19937::result_type>(seed)};
        std::uniform_int_distribution<std::mt19937::result_type> dist(10000,60000);
        int port = 0;
        int count = 0;
        do{
            port = dist(rng);
        }while(tbk::checkPortUsage(port) && count++ < 5);
        auto res = bind(ip,port,f);
        if(!res){
            tbk::error("bind failed, tried many times\n");
            return false;
        }
        return true; 
    }
    bool bind_any(const __callback_type& f={}){
        return bind_any("0.0.0.0",f);
    }
    auto get_bind_ep(){
        return _socket.local_endpoint();
    }
    void join_multicast(const boost::asio::ip::address& multicast_address,const boost::asio::ip::address& if_address = boost::asio::ip::address_v4::any()){
        _socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        _socket.set_option(boost::asio::ip::multicast::enable_loopback(true));
        _socket.set_option(boost::asio::ip::multicast::join_group(multicast_address.to_v4(),if_address.to_v4()));
    }
    void set_interface(const boost::asio::ip::address& if_address){
        auto _if = boost::asio::ip::multicast::outbound_interface(if_address.to_v4());
        // std::cout << "set interface " << _if.ipv4_value_ << std::endl;
        _socket.set_option(_if);
    }
    void set_interface(const int if_num){
        auto _if = boost::asio::ip::multicast::outbound_interface(if_num);
        // std::cout << "set interface " << _if << std::endl;
        _socket.set_option(_if);
    }
    void set_callback(const __callback_type& f){
        _callback = std::bind(f,std::placeholders::_1,std::placeholders::_2);
    }
    // use for sender
    void send_to(const std::string& str,const boost::asio::ip::udp::endpoint& ep){
        send_to(str.c_str(),str.size(),ep);
    }
    void send_to(const void* p,const size_t size,const boost::asio::ip::udp::endpoint& ep){
        _socket.send_to(boost::asio::buffer(p,size),ep);
    }
private:
    bool _bind(const boost::asio::ip::udp::endpoint& _listen_ep,const __callback_type& f = {},const __callback_ep_type& ep_f = {}){
        boost::system::error_code ec;
        _socket.bind(_listen_ep,ec);
        if(ec.value() != 0){
            tbk::error("get error11 {}:{}\n",ec.value(),ec.message());
            return false;
        }
        if(std::getenv("TBK_DEBUG_SOCKET")){
            tbk::status("bind to {}:{}\n",_listen_ep.address().to_string(),_listen_ep.port());
        }
        if(f){
            _callback = std::bind(f,std::placeholders::_1,std::placeholders::_2);
        }
        if(ep_f){
            _ep_callback = std::bind(ep_f,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
        }
        _socket.async_receive_from(boost::asio::buffer(_data,MAX_LENGTH),_received_ep
            , std::bind(&socket::handle_receive_from, this, std::placeholders::_1, std::placeholders::_2)
        );
        _is_running = true;
        return true;
    }
    void handle_receive_from(const boost::system::error_code &ec, size_t bytes_recvd){
        if (ec.value() == 0){
            if(_callback) std::invoke(_callback,_data.data(),bytes_recvd);
            if(_ep_callback) std::invoke(_ep_callback,_received_ep,_data.data(),bytes_recvd);
            _socket.async_receive_from(boost::asio::buffer(_data,MAX_LENGTH),_received_ep
                , std::bind(&socket::handle_receive_from, this, std::placeholders::_1, std::placeholders::_2)
            );
        }else{
            _is_running = false;
            // ignore operation aborted error
            if(ec.value() != boost::asio::error::operation_aborted){
                std::cerr << fmt::format("get error222 {}:{}",ec.value(),ec.message()) << std::endl;
            }
        }
    }
private:
    std::atomic_bool _is_running = false;
    boost::asio::ip::udp::endpoint _received_ep;
    boost::asio::ip::udp::socket _socket;

    const static unsigned int MAX_LENGTH = 65000;
    std::array<char,MAX_LENGTH> _data;
    __callback_type _callback = {};
    __callback_ep_type _ep_callback = {};
};
class ContextManager{
public:
    std::atomic<bool> _running = false;
    ContextManager(boost::asio::io_context* const context):_context(context){}
    ~ContextManager(){
        stop();
        if(_thread.joinable()){
            _thread.join();
        }
    }
    void run(){
        _running = true;
        __io::_()->run();
        __io::_()->reset();
        _running = false;
        // tbk::status("_asio_context exit\n");
    }
    void start(){
        if(_running){
            return;
        }
        std::scoped_lock<std::mutex> lock(_mutex);
        if(!_running){
            if(_thread.joinable()){
                _thread.join();
            }
            _thread = std::thread(&ContextManager::run,this);
        }
    }
    void stop(){
        std::scoped_lock<std::mutex> lock(_mutex);
        if(_running){
            __io::_()->stop();
        }
    }
private:
    std::mutex _mutex;
    std::thread _thread;
    boost::asio::io_context* _context;
};
} // namespace tbk::udp
} // namespace tbk
#endif // __TBK_UDP_SOCKET_H__
