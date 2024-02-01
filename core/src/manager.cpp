#include "tbk/manager.h"
#include "tbk/core.h"
namespace tbk{
manager::manager(const char* _name,const char* _namespace):_uuid(generateUUID()),_pid(getProcessId()),_name(_name),_namespace(_namespace),context_manager(__io::_())
#ifdef TBK_PROCESS_COMM
    ,_etcd_client(etcd::ClientParam(_info_handler,_uuid))
#endif
{
    if(std::getenv("TBK_DEBUG_MANAGER")){
        tbk::status("inside manager address : {},name-{}\n",fmt::ptr(this),_name);
    }
    if(this->_name.empty()){
        tbk::warning("tbk manager name is empty,maybe you need to set one name on ctor.\n");
    }
    context_manager.start();
    // rpc_server.start();

    // tbk::info("wait for rpc server start...\n");
    // while(!rpc_server._rpc_running){
    //     std::this_thread::sleep_for(std::chrono::milliseconds(5));
    // }
    // tbk::info("rpc server start success\n");
    std::string address = "";//rpc_server._address;
    int port = 0;//rpc_server._port;
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.addProcess(ProcessInfo{_uuid,_pid,_name,_namespace,address,port});
    }
    #endif
}
manager::~manager(){
    need_exit = true;
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.removeProcess(ProcessInfo{_uuid,_pid,_name,_namespace,"",0});
    }
    #endif
    // rpc_server.stop();
    context_manager.stop();
    // _t_state_send.join();
}
bool manager::addPub(PublisherBase* ptr,const PublisherInfo pub_info){
    if(std::getenv("TBK_DEBUG_MANAGER")){
        tbk::status("addPub address : {},key-{}\n",fmt::ptr(ptr),pub_info.uuid);
    }
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.addPub(pub_info);
    }
    #endif
    auto res = _info_handler.addPub(ptr,pub_info);
    if(!res.success){
        #ifdef TBK_PROCESS_COMM
        {
            _etcd_client.removePub(pub_info);
        }
        #endif
        tbk::error("addPub failed : {}\n",res.msg);
        return false;
    }
    return true;
}
bool manager::addSub(SubscriberBase* ptr,const SubscriberInfo sub_info){
    if(std::getenv("TBK_DEBUG_MANAGER")){
        tbk::status("addSub address : {},key-{}\n",fmt::ptr(ptr),sub_info.uuid);
    }
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.addSub(sub_info);
    }
    #endif
    auto res = _info_handler.addSub(ptr,sub_info);
    if(!res.success){
        #ifdef TBK_PROCESS_COMM
        {
            _etcd_client.removeSub(sub_info);
        }
        #endif
        tbk::error("addSub failed : {}\n",res.msg);
        return false;
    }
    return true;
}
bool manager::removePub(PublisherBase* ptr,const PublisherInfo pub_info){
    if(std::getenv("TBK_DEBUG_MANAGER")){
        tbk::status("removePub address : {}\n",fmt::ptr(ptr));
    }
    auto res = _info_handler.removePub(ptr,pub_info);
    if(!res.success){
        tbk::error("removePub failed : {}\n",res.msg);
        return false;
    }
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.removePub(pub_info);
    }
    #endif
    return true;
}
bool manager::removeSub(SubscriberBase* ptr,const SubscriberInfo sub_info){
    if(std::getenv("TBK_DEBUG_MANAGER")){
        tbk::status("removeSub address : {}\n",fmt::ptr(ptr));
    }
    auto res = _info_handler.removeSub(ptr,sub_info);
    if(!res.success){
        tbk::error("removeSub failed : {}\n",res.msg);
        return false;
    }
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.removeSub(sub_info);
    }
    #endif
    return true;
}
bool manager::updatePub(PublisherBase* ptr,const PublisherInfo pub_info){
    if(std::getenv("TBK_DEBUG_MANAGER")){
        tbk::status("updatePub address : {},key-{}\n",fmt::ptr(ptr),pub_info.uuid);
    }
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.updatePub(pub_info);
    }
    #endif

    auto res = _info_handler.updatePub(ptr,pub_info);
    if(!res.success){
        tbk::error("updatePub failed : {}\n",res.msg);
        return false;
    }
    return true;
}
bool manager::updateSub(SubscriberBase* ptr,const SubscriberInfo sub_info){
    if(std::getenv("TBK_DEBUG_MANAGER")){
        tbk::status("updateSub address : {},key-{}\n",fmt::ptr(ptr),sub_info.uuid);
    }
    #ifdef TBK_PROCESS_COMM
    {
        _etcd_client.updateSub(sub_info);
    }
    #endif
    auto res = _info_handler.updateSub(ptr,sub_info);
    if(!res.success){
        tbk::error("updateSub failed : {}\n",res.msg);
        return false;
    }
    return true;
}
} // namespace tbk
