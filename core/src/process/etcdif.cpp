#include "etcd/Watcher.hpp"
#include "etcd/KeepAlive.hpp"
#include "tbk/process/etcdif.h"
#include "tbk/protocol/tbk.pb.h"
#include "tbk/log.h"
namespace tbk{
namespace etcd{
Client::Client(const ClientParam& param)
:_prefix(param.prefix)
,_puuid(param.puuid)
,_infoHandler(param.infoHandler)
,_client(
    etcdv3::detail::resolve_etcd_endpoints(param.url),
    fmt::format("{}/{}",param.cert.prefix,param.cert.etcdadm_ca),
    fmt::format("{}/{}",param.cert.prefix,param.cert.etcdadm_cert),
    fmt::format("{}/{}",param.cert.prefix,param.cert.etcdadm_key),
    ""
){
    _keepalive = _client.leasekeepalive(param.ttl);
    _lease_id = _keepalive->Lease();
    init();
    std::scoped_lock lock{_watching_mutex};
    _is_watching = true;
    _watcher = std::make_unique<::etcd::Watcher>(_client,param.prefix,std::bind(&Client::_cb, this, std::placeholders::_1),true);
}
Client::~Client(){
    std::scoped_lock lock{_watching_mutex};
    _is_watching = false;
    _watcher->Cancel();
}
void Client::init(){
    auto resp = _client.ls(_prefix);
    auto self_prefix = fmt::format("{}/{}",_prefix,_puuid);
    int index = -1;
    for(auto const& kv: resp.keys()){
        index++;
        if(kv.starts_with(self_prefix)){
            continue;
        }
        auto keys = decodeKey(_prefix,kv);
        HandleResult handleRes;
        if(keys.size() == 1){
             handleRes = _infoHandler.addOuter(decodeProcess(resp.value(index).as_string()));
        }else if(keys.size() == 3){
            if(keys[1] == "pubs"){
                handleRes = _infoHandler.updateOuterPub(decodePub(resp.value(index).as_string()));
            }else if(keys[1] == "subs"){
                handleRes = _infoHandler.updateOuterSub(decodeSub(resp.value(index).as_string()));
            }else{
                tbk::error("unknown key:{}\n",kv);
            }
        }
        if(!handleRes.success){
            tbk::error("err in handle:{},{}\n",handleRes.msg,kv);
        }
    }
}
void Client::_cb(::etcd::Response const& resp){
    std::scoped_lock lock{_watching_mutex};
    if(!_is_watching){
        return;
    }
    if (resp.error_code()) {
        std::cout << resp.error_code() << ": " << resp.error_message() << std::endl;
        return;
    }
    auto self_prefix = fmt::format("{}/{}",_prefix,_puuid);
    for (auto const &ev: resp.events()) {
        // fix etcd-cpp-apiv3 bug : watcher event not filted by prefix
        if (!ev.prev_kv().key().starts_with(_prefix) && !ev.kv().key().starts_with(_prefix)) {
            continue;
        }
        if(ev.prev_kv().key().starts_with(self_prefix) || ev.kv().key().starts_with(self_prefix)){
            continue;
        }

        auto keys = decodeKey(_prefix,ev.kv().key());
        HandleResult handleRes;
        if(keys.size() == 1){
            if(ev.event_type() == ::etcd::Event::EventType::DELETE_){
                handleRes = _infoHandler.removeOuter(decodeProcess(ev.prev_kv().as_string()));
            }else if(ev.event_type() == ::etcd::Event::EventType::PUT){
                handleRes = _infoHandler.addOuter(decodeProcess(ev.kv().as_string()));
            }
        }else if(keys.size() == 3){
            if(keys[1] == "pubs"){
                if(ev.event_type() == ::etcd::Event::EventType::DELETE_){
                    handleRes = _infoHandler.removeOuterPub(decodePub(ev.prev_kv().as_string()));
                }else if(ev.event_type() == ::etcd::Event::EventType::PUT){
                    handleRes = _infoHandler.updateOuterPub(decodePub(ev.kv().as_string()));
                }
            }else if(keys[1] == "subs"){
                if(ev.event_type() == ::etcd::Event::EventType::DELETE_){
                    handleRes = _infoHandler.removeOuterSub(decodeSub(ev.prev_kv().as_string()));
                }else if(ev.event_type() == ::etcd::Event::EventType::PUT){
                    handleRes = _infoHandler.updateOuterSub(decodeSub(ev.kv().as_string()));
                }
            }else{
                tbk::error("unknown key:{}\n",ev.kv().key());
            }
        }else{
            tbk::error("unknown key:{}\n",ev.kv().key());
        }
        if(!handleRes.success){
            tbk::error("err in handle:{},{},{},{}\n",int(ev.event_type()),handleRes.msg,ev.kv().key(),keys[0]);
        }
    }
}
bool Client::addProcess(const ProcessInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd addProcess:{}-{}:{}\n",info.uuid,info.ip,info.port);
    }
    auto res = _client.add(fmt::format("{}/{}",_prefix,info.uuid), encodeProcess(info),_lease_id);
    if (res.error_code()) {
        tbk::error("err in addProcess:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
bool Client::removeProcess(const ProcessInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd removeProcess:{}\n",info.uuid);
    }
    auto res = _client.rm(fmt::format("{}/{}",_prefix,info.uuid));
    if (res.error_code()) {
        tbk::error("err in removeProcess:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
bool Client::addPub(const PublisherInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd addPub:{}\n",info.uuid);
    }
    auto res = _client.add(fmt::format("{}/{}/pubs/{}",_prefix,info.puuid,info.uuid), encodePub(info),_lease_id);
    if (res.error_code()) {
        tbk::error("err in addPub:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
bool Client::addSub(const SubscriberInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd addSub:{}\n",info.uuid);
    }
    auto res = _client.add(fmt::format("{}/{}/subs/{}",_prefix,info.puuid,info.uuid), encodeSub(info),_lease_id);
    if (res.error_code()) {
        tbk::error("err in addSub:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
bool Client::removePub(const PublisherInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd removePub:{}\n",info.uuid);
    }
    auto res = _client.rm(fmt::format("{}/{}/pubs/{}",_prefix,info.puuid,info.uuid));
    if (res.error_code()) {
        tbk::error("err in removePub:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
bool Client::removeSub(const SubscriberInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd removeSub:{}\n",info.uuid);
    }
    auto res = _client.rm(fmt::format("{}/{}/subs/{}",_prefix,info.puuid,info.uuid));
    if (res.error_code()) {
        tbk::error("err in removeSub:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
bool Client::updatePub(const PublisherInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd updatePub:{}\n",info.uuid);
    }
    auto res = _client.modify(fmt::format("{}/{}/pubs/{}",_prefix,info.puuid,info.uuid), encodePub(info),_lease_id);
    if (res.error_code()) {
        tbk::error("err in updatePub:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
bool Client::updateSub(const SubscriberInfo& info){
    if(getenv("TBK_DEBUG_ETCD")){
        tbk::info("etcd updateSub:{}\n",info.uuid);
    }
    auto res = _client.modify(fmt::format("{}/{}/subs/{}",_prefix,info.puuid,info.uuid), encodeSub(info),_lease_id);
    if (res.error_code()) {
        tbk::error("err in updateSub:{},{}-{}\n",info.uuid,res.error_code(),res.error_message());
        return false;
    }
    return true;
}
std::string encodeProcess(const ProcessInfo& info){
    pb::State pb_state;
    pb::EndPoint* pb_ep = pb_state.mutable_ep();
    pb_ep->set_address(info.ip);
    pb_ep->set_port(info.port);
    pb_state.set_uuid(info.uuid);
    pb_state.set_pid(info.pid);
    pb_state.set_node_name(info.name);
    pb_state.set_node_ns(info.ns);
    return pb_state.SerializeAsString();
}
ProcessInfo decodeProcess(const std::string& data){
    pb::State pb_state;
    pb_state.ParseFromString(data);
    ProcessInfo info;
    info.ip = pb_state.ep().address();
    info.port = pb_state.ep().port();
    info.uuid = pb_state.uuid();
    info.pid = pb_state.pid();
    info.name = pb_state.node_name();
    info.ns = pb_state.node_ns();
    return std::move(info);
}

std::string encodePub(const PublisherInfo& info){
    pb::Publisher pb_pub;
    pb_pub.set_ip(info.ip);
    pb_pub.set_puuid(info.puuid);
    pb_pub.set_pid(info.pid);
    pb_pub.set_uuid(info.uuid);
    pb_pub.set_msg_name(info.msg_name);
    pb_pub.set_name(info.name);
    pb_pub.set_node_name(info.node_name);
    pb_pub.set_node_ns(info.node_ns);
    pb_pub.set_ns(info.ns);
    for(auto& sub_info: info.subs){
        pb::Subscriber* pb_sub = pb_pub.add_subs();
        pb::EndPoint* pb_ep = pb_sub->mutable_ep();
        pb_ep->set_address(sub_info.ip);
        pb_ep->set_port(sub_info.port);
        pb_sub->set_puuid(sub_info.puuid);
        pb_sub->set_pid(sub_info.pid);
        pb_sub->set_uuid(sub_info.uuid);
        pb_sub->set_msg_name(sub_info.msg_name);
        pb_sub->set_name(sub_info.name);
        pb_sub->set_node_name(sub_info.node_name);
        pb_sub->set_node_ns(sub_info.node_ns);
    }
    std::string pb_str;
    pb_pub.SerializeToString(&pb_str);
    return std::move(pb_str);
}
PublisherInfo decodePub(const std::string& data){
    pb::Publisher pb_pub;
    pb_pub.ParseFromString(data);
    PublisherInfo info;
    info.ip = pb_pub.ip();
    info.puuid = pb_pub.puuid();
    info.pid = pb_pub.pid();
    info.uuid = pb_pub.uuid();
    info.msg_name = pb_pub.msg_name();
    info.name = pb_pub.name();
    info.node_name = pb_pub.node_name();
    info.node_ns = pb_pub.node_ns();
    info.ns = pb_pub.ns();
    for(int i=0;i<pb_pub.subs_size();i++){
        SubscriberInfo sub_info;
        sub_info.ip = pb_pub.subs(i).ep().address();
        sub_info.port = pb_pub.subs(i).ep().port();
        sub_info.puuid = pb_pub.subs(i).puuid();
        sub_info.pid = pb_pub.subs(i).pid();
        sub_info.uuid = pb_pub.subs(i).uuid();
        sub_info.msg_name = pb_pub.subs(i).msg_name();
        sub_info.name = pb_pub.subs(i).name();
        sub_info.node_name = pb_pub.subs(i).node_name();
        sub_info.node_ns = pb_pub.subs(i).node_ns();
        info.addSubs(std::move(sub_info));
    }
    return std::move(info);
}
std::string encodeSub(const SubscriberInfo& info){
    pb::Subscriber pb_sub;
    pb::EndPoint* pb_ep = pb_sub.mutable_ep();
    pb_ep->set_address(info.ip);
    pb_ep->set_port(info.port);
    pb_sub.set_puuid(info.puuid);
    pb_sub.set_pid(info.pid);
    pb_sub.set_uuid(info.uuid);
    pb_sub.set_msg_name(info.msg_name);
    pb_sub.set_name(info.name);
    pb_sub.set_node_name(info.node_name);
    pb_sub.set_node_ns(info.node_ns);
    pb_sub.set_ns(info.ns);
    std::string pb_str;
    pb_sub.SerializeToString(&pb_str);
    return std::move(pb_str);
}
SubscriberInfo decodeSub(const std::string& data){
    pb::Subscriber pb_sub;
    pb_sub.ParseFromString(data);
    SubscriberInfo info;
    info.ip = pb_sub.ep().address();
    info.port = pb_sub.ep().port();
    info.puuid = pb_sub.puuid();
    info.pid = pb_sub.pid();
    info.uuid = pb_sub.uuid();
    info.msg_name = pb_sub.msg_name();
    info.name = pb_sub.name();
    info.node_name = pb_sub.node_name();
    info.node_ns = pb_sub.node_ns();
    info.ns = pb_sub.ns();
    return std::move(info);
}
std::vector<std::string> decodeKey(const std::string& _prefix,const std::string& keyWithPrefix){
    std::string key = keyWithPrefix.substr(_prefix.size()+1);
    std::vector<std::string> res;
    std::string::size_type pos1,pos2;
    pos2 = key.find('/');
    pos1 = 0;
    while(std::string::npos != pos2){
        res.push_back(key.substr(pos1, pos2-pos1));
        pos1 = pos2 + 1;
        pos2 = key.find('/', pos1);
    }
    if(pos1 != key.length())
        res.push_back(key.substr(pos1));
    return std::move(res);
}

ParamsServer::ParamsServer(const ParamsServerParam& p)
:_prefix(p.prefix)
,__cb(p.cb)
,_client(
    etcdv3::detail::resolve_etcd_endpoints(p.url),
    fmt::format("{}/{}",p.cert.prefix,p.cert.etcdadm_ca),
    fmt::format("{}/{}",p.cert.prefix,p.cert.etcdadm_cert),
    fmt::format("{}/{}",p.cert.prefix,p.cert.etcdadm_key),
    ""
){
    _watcher = std::make_unique<::etcd::Watcher>(_client,p.prefix,std::bind(&ParamsServer::_cb, this, std::placeholders::_1),true);
}
void ParamsServer::_cb(::etcd::Response const& resp){
    if (resp.error_code()) {
        std::cout << resp.error_code() << ": " << resp.error_message() << std::endl;
        return;
    }
    for (auto const &ev: resp.events()) {
        // fix etcd-cpp-apiv3 bug : watcher event not filted by prefix
        if (!ev.prev_kv().key().starts_with(_prefix) && !ev.kv().key().starts_with(_prefix)) {
            continue;
        }
        std::string key = ev.kv().key().substr(_prefix.size()+1);
        if(__cb){
            __cb(key,ev.kv().as_string());
        }
    }
}
void ParamsServer::set(const std::string& name,const std::string& value){
    _client.set(_prefix+"/"+name,value);
}
ParamsServer::Result ParamsServer::get(const std::string& name){
    auto resp = _client.get(_prefix+"/"+name);
    if(resp.error_code()){
        return {false,""};
    }
    return {true,resp.value().as_string()};
}
} // namespace tbk::etcd
} // namespace tbk
