#ifndef __TBKCORE_H__
#define __TBKCORE_H__
#include <string>
#include <set>
#include <typeinfo>
#include <iostream>
#include <atomic>

#include "tbk/utils/threadpool.h"
#include "tbk/utils/singleton.h"
#include "tbk/config.h"
#include "tbk/token.h"
#include "tbk/semadata.h"
#include "tbk/log.h"
#include "tbk/type.h"
#include "tbk/socket.h"

namespace tbk{
using __callback_type = tbk::type::callback_type;
using __socket_callback_type = tbk::type::socket_callback_type;
class SubscriberBase;
class Publisher;
class Pool:public Singleton<Pool>,public ThreadPool{
public:
    Pool():ThreadPool(tbk::config::threadpool_nums){}
    friend SubscriberBase;
};
class SubscriberBase{
    friend class PublisherBase;
    friend class InfoHandler;
public:
    SubscriberBase(const unsigned int,const std::string& cs,const std::string& name,const std::string& msg_name,const __callback_type& f = {});
    SubscriberBase(const unsigned int,const std::string& name,const std::string& msg_name,const __callback_type& f = {});
    ~SubscriberBase();
    std::string name() const{
        return _info.name;
    }
    std::string msg_name() const{
        return _info.msg_name;
    }
    // should be used when there's no callback functions
    void get(Data& data){
        _data->pop(data);
    }
    void get(Data* data=nullptr){
        _data->pop(data);
    }
    bool try_get(Data& data){
        auto res = _data->try_pop(data);
        return res;
    }
    bool setCommLevel(const CommLevel& level, const bool report = true){
        _commLevel = level;
        if(_commLevel.load() > CommLevel::Process){
            setSocket(report);
        }else{
            unsetSocket(report);
        }
        return true;
    }
    bool socketReady(){
        std::scoped_lock<std::mutex> lock(_socket_mutex);
        return _has_socket;
    }
    tbk::udp::endpoint getBindEp(){
        std::scoped_lock<std::mutex> lock(_socket_mutex);
        if(_socket.is_running()){
            return _socket.get_bind_ep();
        }
        return {};
    }
    std::string getBindAddress(){
        return getBindEp().address().to_string();
    }
    unsigned short getBindPort(){
        return getBindEp().port();
    }
    CommLevel commLevel() const{
        return _commLevel.load();
    }
protected:
    void _socket_cb(const void* data,size_t size){
        this->add_task(data,size);
    }
    bool add_task(const void* data,size_t size){
        std::scoped_lock lock{_add_task_mutex};
        if (need_exit) return false;
        if(_callback){
            tbk::Data _data;
            _data.store(data,size);
            Pool::_()->enqueue(_callback,std::move(_data));
        }else{
            this->_data->store(data,size);
        }
        return true;
    }
    std::atomic_bool need_exit = false;
    std::mutex _add_task_mutex;
    std::unique_ptr<SemaData> _data;
    __callback_type _callback = {};
    __socket_callback_type _socket_callback = {};


    bool setSocket(const bool report = true);
    bool unsetSocket(const bool report = true);
    std::atomic<CommLevel> _commLevel = CommLevel::Default;
    tbk::udp::socket _socket;
    std::mutex _socket_mutex;
    std::atomic_bool _has_socket = false;
    SubscriberInfo _info;
};
template<unsigned int buffer_size=2>
class Subscriber:public SubscriberBase{
public:
    Subscriber(const std::string& name,const __callback_type& f = {}):Subscriber(buffer_size,"",name+"_subcriber",name,f){}
    Subscriber(const std::string& name,const std::string& msg_name,const __callback_type& f = {}):SubscriberBase(buffer_size,"",name,msg_name,f){}
    Subscriber(const std::string& cs,const std::string& name,const std::string& msg_name,const __callback_type& f = {}):SubscriberBase(buffer_size,cs,name,msg_name,f){}
    Subscriber(const int _buffer_size,const std::string& name,const std::string& msg_name,const __callback_type& f = {}):SubscriberBase(_buffer_size,"",name,msg_name,f){}
    Subscriber(const int _buffer_size,const std::string& cs,const std::string& name,const std::string& msg_name,const __callback_type& f = {}):SubscriberBase(_buffer_size,cs,name,msg_name,f){}
};
class PublisherBase{
    friend class InfoHandler;
public:
    PublisherBase(const std::string& cs,const std::string& name,const std::string& msg_name);
    PublisherBase(const std::string& name,const std::string& msg_name);
    ~PublisherBase();
    virtual void publish(const void* data = nullptr, const unsigned long size = 0){
        if(getenv("TBK_DEBUG_CORE")){
            tbk::log("{}-trigger publish -> receiver nums:{}+{}\n",name(),_subscribers.size(),_u_subscribers.size());
        }
        std::shared_lock s_lock(_mutex_subscriber);
        for(auto& s:_subscribers){
            s->add_task(data,size);
        }
        {
            std::shared_lock<std::shared_mutex> lock(_mutex_u_subscriber);
            for(auto& ep:_u_subscribers){
                _socket.send_to(data,size,ep);
            }
        }
    }
    std::string name() const{
        return _info.name;
    }
    std::string msg_name() const{
        return _info.msg_name;
    }
    virtual void publish(const Data& data){
        this->publish(data.data(),data.size());
    }
    virtual void publish(const std::string &data){
        this->publish(data.data(),data.length());
    }
    bool setCommLevel(const CommLevel& level){
        _commLevel = level;
        return true;
    }
    CommLevel commLevel() const{
        return _commLevel.load();
    }
public:
    void link(SubscriberBase* subs){
        link(true,subs);
    }
    void link(bool tell_manager,SubscriberBase* subs){
        std::unique_lock u_lock(this->_mutex_subscriber);
        _subscribers.insert(subs);
        auto res = _info.addSubs(subs->_info);
        if(res && tell_manager){
            update2manager();
        }
        if(getenv("TBK_DEBUG_CORE")){
            tbk::log("Publisher link() : {}\n",fmt::ptr(subs));
        }
    }
    void unlink(SubscriberBase* subs){
        unlink(true,subs);
    }
    void unlink(bool tell_manager,SubscriberBase* subs){
        std::unique_lock u_lock(this->_mutex_subscriber);
        _subscribers.erase(subs);
        auto res = _info.removeSubs(subs->_info);
        if(res && tell_manager){
            update2manager();
        }
        if(getenv("TBK_DEBUG_CORE")){
            tbk::log("Publisher unlink() : {}\n",fmt::ptr(subs));
        }
    }
    void link_u(tbk::SubscriberInfo subs){
        link_u(true,subs);
    }
    void link_u(bool tell_manager,tbk::SubscriberInfo subs){
        std::unique_lock u_lock(this->_mutex_u_subscriber);
        _u_subscribers.insert(tbk::udp::endpoint(tbk::udp::address::from_string(subs.ip),subs.port));
        auto res = _info.addSubs(subs);
        if(res && tell_manager){
            update2manager();
        }
        if(getenv("TBK_DEBUG_CORE")){
            tbk::log("Publisher link_u() : {}:{}\n",subs.ip,subs.port);
        }
    }
    void unlink_u(tbk::SubscriberInfo subs){
        unlink_u(true,subs);
    }
    void unlink_u(bool tell_manager, tbk::SubscriberInfo subs){
        std::unique_lock u_lock(this->_mutex_u_subscriber);
        _u_subscribers.erase(tbk::udp::endpoint(tbk::udp::address::from_string(subs.ip),subs.port));
        auto res = _info.removeSubs(subs);
        if(res && tell_manager){
            update2manager();
        }
        if(getenv("TBK_DEBUG_CORE")){
            tbk::log("Publisher unlink_u() {}:{}\n",subs.ip,subs.port);
            tbk::log("Publisher after unlink_u() : size-{}\n",_u_subscribers.size());
        }
    }
    void update2manager();
protected:
    std::set<SubscriberBase*> _subscribers = {};
    std::shared_mutex _mutex_subscriber;

    std::atomic<CommLevel> _commLevel = CommLevel::Default;
    tbk::udp::socket _socket;
    std::set<tbk::udp::endpoint> _u_subscribers = {};
    std::shared_mutex _mutex_u_subscriber;
    PublisherInfo _info;
};
class Publisher:public PublisherBase{
public:
    Publisher(const std::string& name):Publisher(name+"_publisher",name){}
    Publisher(const std::string& name,const std::string& msg_name):PublisherBase(name,msg_name){}
    Publisher(const std::string& cs,const std::string& name,const std::string& msg_name):PublisherBase(cs,name,msg_name){}
};
} // namespace tbk

#endif // __TBKCORE_H__
