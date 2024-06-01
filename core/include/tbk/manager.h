#ifndef __TBK_MANAGER_H__
#define __TBK_MANAGER_H__
#include <atomic>
#include <memory>
#include <cstdlib>
#include <ctime>
#include "tbk/meta.h"
#include "tbk/config.h"
#include "tbk/utils/rate.h"
#include "tbk/log.h"
#include "tbk/socket.h"
#include "tbk/setting.h"
#include "tbk/posix.h"
#include "tbk/protocol/tbk.pb.h"

#ifdef TBK_PROCESS_COMM
    // #include "tbk/process/rpc.h"
    #include "tbk/process/etcdif.h"
#endif

namespace tbk{
class PublisherBase;
class SubscriberBase;
class manager:public tbk::Singleton<manager>{
    // friend class rpc::zrpcImpl;
public:
    manager(const char* _name="",const char* _namespace="");
    ~manager();
    int pid()const{ return _pid; }
    std::string uuid()const{ return _uuid; }
    std::string node_name()const{ return _name; }
    std::string node_ns()const{ return _namespace; }
    // apis for inner use
    bool addPub(PublisherBase*, const PublisherInfo);
    bool addSub(SubscriberBase*, const SubscriberInfo);
    bool removePub(PublisherBase*, const PublisherInfo);
    bool removeSub(SubscriberBase*, const SubscriberInfo);
    bool updatePub(PublisherBase*, const PublisherInfo);
    bool updateSub(SubscriberBase*, const SubscriberInfo);
    void udpRun(){ context_manager.start(); }
private:
    bool _rpc_api_add(const std::string& res_uuid, const SubscriberInfo& sub_info);
    bool _rpc_api_del(const std::string& res_uuid, const SubscriberInfo& sub_info);
    bool _rpc_api_get(const std::string& req_uuid,const std::string& res_uuid,const std::string& linker_uuid);
    const std::string _name;
    const std::string _namespace;
    const int _pid;
    const std::string _uuid;

    std::thread _t_state_send;
    std::atomic<bool> need_exit = false;

    udp::ContextManager context_manager;

    InfoHandler _info_handler;
    #ifdef TBK_PROCESS_COMM
        // rpc::Server rpc_server;
        etcd::Client _etcd_client;
    #endif
};
} // namespace tbk
#endif // __TBK_MANAGER_H__
