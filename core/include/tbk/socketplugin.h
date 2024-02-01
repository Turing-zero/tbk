#ifndef UDPPLUGIN_H
#define UDPPLUGIN_H

#include "tbk/data.h"
#include "tbk/socket.h"

namespace tbk{
namespace udp{
template<tbk::concepts::Serializable T>
class Plugin{
    using _callback_type = std::function<void(const T&)>;
    using _raw_callback_type = std::function<void(const void*,size_t)>;
    Plugin(const tbk::udp::endpoint& ep,const char* multicast_address,const _callback_type& f = {}, const _raw_callback_type& f2 = {}):_callback(f),_raw_callback(f2){
        if(multicast_address)
            _socket.join_multicast(tbk::udp::address::from_string(multicast_address));
        if(_callback)
            _socket.bind(ep,std::bind(&Plugin::_socket_cb,this,std::placeholders::_1,std::placeholders::_2));
        else if(_raw_callback)
            _socket.bind(ep,_raw_callback);
    }
public:
    Plugin(){};
    Plugin(const char* multicast_address){
        _socket.join_multicast(tbk::udp::address::from_string(multicast_address));
    }
    Plugin(int port,const _callback_type& f={}):Plugin(tbk::udp::endpoint(tbk::udp::address::from_string("0.0.0.0"),port),nullptr,f,{}){}
    Plugin(int port,const _raw_callback_type& f={}):Plugin(tbk::udp::endpoint(tbk::udp::address::from_string("0.0.0.0"),port),nullptr,{},f){}
    Plugin(const tbk::udp::endpoint& ep,const _callback_type& f = {}):Plugin(ep,nullptr,f,{}){}
    void send(const T& t,const tbk::udp::endpoint& ep){
        size_t size = t.ByteSizeLong();
        _data.resize(size);
        t.SerializeToArray(_data.ptr(),size);
        _socket.send_to(_data.data(),size,ep);
    }
    void send(const T& t,const int port){
        send(t,tbk::udp::endpoint(tbk::udp::address::from_string("0.0.0.0"),port));
    }
    void sendData(const void* ptr, const size_t size,const tbk::udp::endpoint& ep){
        _socket.send_to(ptr,size,ep);
    }
    void sendData(const void* ptr, const size_t size,const int port){
        _socket.send_to(ptr,size,tbk::udp::endpoint(tbk::udp::address::from_string("0.0.0.0"),port));
    }
private:
    void _socket_cb(const void* p,size_t lens){
        if(_callback){
            t.ParseFromArray(p,lens);
            _callback(t);
        }
    }
    T t;
    tbk::udp::socket _socket;
    tbk::Data _data;
    _callback_type _callback;
    _raw_callback_type _raw_callback;
};
} // namespace tbk::udp;
} // namespace tbk;
#endif // UDPPLUGIN_H
