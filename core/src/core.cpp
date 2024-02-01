#include "tbk/core.h"
#include "tbk/manager.h"
namespace tbk{
SubscriberBase::SubscriberBase(const unsigned int buffer_size,const std::string& name,const std::string& msg_name,const __callback_type& f):SubscriberBase(buffer_size,"",name,msg_name,f){}
SubscriberBase::SubscriberBase(const unsigned int buffer_size,const std::string& cs,const std::string& name,const std::string& msg_name,const __callback_type& f):_data(std::make_unique<SemaData>(buffer_size))
    ,_info(
        "",
        0,
        tbk::manager::_()->uuid(),
        tbk::manager::_()->pid(),
        tbk::manager::_()->node_name(),
        tbk::manager::_()->node_ns(),
        cs,
        generateUUID(),
        name,
        msg_name,
        this,
        InfoFrom::SELF
    ){
    if(getenv("TBK_DEBUG_CORE")){
        tbk::log("Subscriber Ctor : {}\n",fmt::ptr(this));
    }
    _socket_callback = std::bind(&SubscriberBase::_socket_cb, this, std::placeholders::_1, std::placeholders::_2);
    setCommLevel(CommLevel::Localhost, false);
    if(f){
        _callback = std::bind(f,std::placeholders::_1);
    }
    tbk::manager::_()->addSub(this,_info);
}
SubscriberBase::~SubscriberBase(){
    if(getenv("TBK_DEBUG_CORE")){
        tbk::log("Subscriber Dtor : {}\n",fmt::ptr(this));
    }
    need_exit = true;
    std::scoped_lock lock{_add_task_mutex};
    tbk::manager::_()->removeSub(this,_info);
    if(_has_socket)
        unsetSocket(false);
}
bool SubscriberBase::setSocket(const bool report){
    #ifdef TBK_PROCESS_COMM
    {
        std::scoped_lock<std::mutex> lock(_socket_mutex);
        if(!_socket.is_running()){
            _socket.bind_any(_socket_callback);
            if(getenv("TBK_DEBUG_CORE")){
                // output asio endpoint address and port
                tbk::log("socket binded to: {}:{}\n",_socket.get_bind_ep().address().to_string(),_socket.get_bind_ep().port());
            }
        }
        _has_socket = true;
        auto ep = _socket.get_bind_ep();
        _info.ip = ep.address().to_string();
        _info.port = ep.port();
        // need fix , keep asio context_io running
        tbk::manager::_()->udpRun();
        if(report)
            tbk::manager::_()->updateSub(this,_info);
    }
    #endif
    return true;
}
bool SubscriberBase::unsetSocket(const bool report){
    #ifdef TBK_PROCESS_COMM
    {
        std::scoped_lock<std::mutex> lock(_socket_mutex);
        _socket.close();
        _has_socket = false;
        _info.ip = "";
        _info.port = 0;
        if(report)
            tbk::manager::_()->updateSub(this,_info);
    }
    #endif
    return true;
}

PublisherBase::PublisherBase(const std::string& name,const std::string& msg_name):PublisherBase("",name,msg_name){}
PublisherBase::PublisherBase(const std::string& cs,const std::string& name,const std::string& msg_name)
    :_info(
        "",
        tbk::manager::_()->uuid(),
        tbk::manager::_()->pid(),
        tbk::manager::_()->node_name(),
        tbk::manager::_()->node_ns(),
        cs,
        generateUUID(),
        name,
        msg_name,
        this,
        InfoFrom::SELF
    ){
    if(getenv("TBK_DEBUG_CORE")){
        tbk::log("Publisher Ctor : {}\n",fmt::ptr(this));
    }
    tbk::manager::_()->addPub(this,_info);
}
PublisherBase::~PublisherBase(){
    if(getenv("TBK_DEBUG_CORE")){
        tbk::log("Publisher Dtor : {}\n",fmt::ptr(this));
    }
    tbk::manager::_()->removePub(this,_info);
}
void PublisherBase::update2manager(){
    tbk::manager::_()->updatePub(this,_info);
}
} // namespace tbk
