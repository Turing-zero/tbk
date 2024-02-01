#ifndef __TBK_ETCD_IF_H__
#define __TBK_ETCD_IF_H__
#include <etcd/SyncClient.hpp>
#include <etcd/Watcher.hpp>
#include "tbk/type.h"
namespace tbk{
namespace etcd{
std::vector<std::string> decodeKey(const std::string& prefix,const std::string& keyWithPrefix);
std::string encodeProcess(const ProcessInfo&);
ProcessInfo decodeProcess(const std::string&);
std::string encodePub(const PublisherInfo&);
PublisherInfo decodePub(const std::string&);
std::string encodeSub(const SubscriberInfo&);
SubscriberInfo decodeSub(const std::string&);
struct ClientParam{
    InfoHandler& infoHandler;
    std::string puuid;
    std::string prefix = "/tbk/ps";
    std::string url = "http://127.0.0.1:2379";
    const int ttl = 1;
};
class Client{
public:
    Client(const ClientParam& param);
    ~Client();
    bool addProcess(const ProcessInfo&);
    bool removeProcess(const ProcessInfo&);
    bool addPub(const PublisherInfo&);
    bool addSub(const SubscriberInfo&);
    bool removePub(const PublisherInfo&);
    bool removeSub(const SubscriberInfo&);
    bool updatePub(const PublisherInfo&);
    bool updateSub(const SubscriberInfo&);
private:
    void init();
    void _cb(::etcd::Response const& res);

    const std::string _prefix;
    const std::string _puuid;
    InfoHandler& _infoHandler;

    ::etcd::SyncClient _client;
    std::unique_ptr<::etcd::Watcher> _watcher;
    std::mutex _watching_mutex;
    std::atomic_bool _is_watching;
    std::shared_ptr<::etcd::KeepAlive> _keepalive;
    int64_t _lease_id;
    // TODO watcher need reconnect
};
struct ParamsServerParam{
    std::string prefix = "/tbk/params/global";
    std::string url = "http://127.0.0.1:2379";
    tbk::type::param_callback_type cb;
    ParamsServerParam() = default;
    ParamsServerParam(const tbk::type::param_callback_type& cb):cb(cb){}
};
class ParamsServer{
public:
    struct Result{
        bool success = false;
        std::string value = "";
        Result() = default;
        Result(bool success,const std::string& value):success(success),value(value){}
    };
    ParamsServer(const ParamsServerParam& param);
    void set(const std::string& name,const std::string& value);
    Result get(const std::string& name);
private:
    void _cb(::etcd::Response const&);
    tbk::type::param_callback_type __cb = {};
    const std::string _prefix;
    ::etcd::SyncClient _client;
    std::unique_ptr<::etcd::Watcher> _watcher;
};
} // namespace tbk::etcd
} // namespace tbk
#endif // __TBK_ETCD_IF_H__
