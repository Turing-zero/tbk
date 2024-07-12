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
    ),_param_commLevel(
        fmt::format("__cl__/{}_{}_{}:{}",
            tbk::manager::_()->node_ns(),
            tbk::manager::_()->node_name(),
            name,msg_name),
        CommLevel::Localhost,
        [this](const CommLevel& prev,const CommLevel& value){
            tbk::log("SubscriberBase::setCommLevel : {} -> {}/{}\n",_param_commLevel.name(),tbk::param::convert<CommLevel>::from(prev),tbk::param::convert<CommLevel>::from(value));
            setCommLevel(value);
        }
    ){
    if(getenv("TBK_DEBUG_CORE")){
        tbk::log("Subscriber Ctor : {}\n",fmt::ptr(this));
    }
    _socket_callback = std::bind(&SubscriberBase::_msgwrap_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
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
            _socket.bind_any(tbk::getClusterIP(),_socket_callback);
            if(getenv("TBK_DEBUG_CORE")){
                // output asio endpoint address and port
                tbk::log("socket binded to: {}:{}\n",_socket.get_bind_ep().address().to_string(),_socket.get_bind_ep().port());
            }
        }
        _has_socket = true;
        auto ep = _socket.get_bind_ep();
        _info.ip = ep.address().to_string();
        _info.port = ep.port();
        tbk::log("SubscriberBase::setSocket : {}:{}\n",_info.ip,_info.port);
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

void SubscriberBase::_msgwrap_cb(const boost::asio::ip::udp::endpoint& ep,const void* data,size_t size){
    _msg_pack.ParseFromArray(data,size);
    std::string _uuid = _msg_pack.uuid();
    _monitor.addMsg(_uuid,{
        std::chrono::microseconds(_msg_pack.send_time()),
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()),
        _msg_pack.index()
    });
    if(getenv("TBK_DEBUG_CORE")){
        tbk::log("current state : frq-{},hr-{}\n",_monitor.getFreq(_uuid),_monitor.getHitRate(_uuid));
    }
    
    this->add_task(_msg_pack.data().c_str(),_msg_pack.data().size());
}
bool SubscriberBase::add_task(const void* data,size_t size){
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
    ),_param_commLevel(
        fmt::format("__cl__/{}_{}_{}:{}",
            tbk::manager::_()->node_ns(),
            tbk::manager::_()->node_name(),
            name,msg_name),
        CommLevel::Default,
        [this](const CommLevel&,const CommLevel& value){
            tbk::log("PublisherBase::setCommLevel : {} -> {}\n",_param_commLevel.name(),tbk::param::convert<CommLevel>::from(value));
            setCommLevel(value);
        }
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
void PublisherBase::publish(const void* data, const unsigned long size){
    if(getenv("TBK_DEBUG_CORE")){
        tbk::log("{}-trigger publish -> receiver nums:{}+{}\n",name(),_subscribers.size(),_u_subscribers.size());
    }
    {
        std::shared_lock s_lock(_mutex_subscriber);
        for(auto& s:_subscribers){
            s->add_task(data,size);
        }
    }
    {
        std::string data_str((char*)data,size);
        _msg_pack.set_send_time((std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch())).count());
        _msg_pack.set_index(_msg_index++);
        _msg_pack.set_uuid(_info.uuid);
        // _msg_pack.set_type(tbk::pb::MsgWrap::ProtocolType::BYTES); // TODO
        _msg_pack.set_allocated_data(&data_str);
        std::shared_lock<std::shared_mutex> lock(_mutex_u_subscriber);
        for(auto& ep:_u_subscribers){
            _socket.send_to(_msg_pack.SerializeAsString(),ep);
        }
        _msg_pack.release_data();
        _msg_pack.Clear();
    }
}
void PublisherBase::update2manager(){
    tbk::manager::_()->updatePub(this,_info);
}
} // namespace tbk
