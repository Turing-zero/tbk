#ifndef __TBK_META_TYPES_H__
#define __TBK_META_TYPES_H__
#include <functional>
#include <string>
#include <map>
#include <set>
#include <mutex>
#include <fmt/format.h>
#include <boost/asio.hpp>
#include "tbk/meta.h"
#include "tbk/data.h"
#include "paramconvert.h"
namespace tbk{
// TODO use
enum class InfoFrom{
    UNASSIGNED,
    SELF,
    OUTER
};
enum class CommLevel{
    Manual = 0,
    Default = 100,
    Process = 100,
    Localhost = 200,
    LocalNetwork = 300,
};
namespace param{
template<>
struct convert<CommLevel>{
    static CommLevel to(const std::string& s){
        if(s == "Manual"){
            return CommLevel::Manual;
        }else if(s == "Process"){
            return CommLevel::Process;
        }else if(s == "Localhost"){
            return CommLevel::Localhost;
        }else if(s == "LocalNetwork"){
            return CommLevel::LocalNetwork;
        }
        return CommLevel::Default;
    }
    static std::string from(const CommLevel& v){
        switch(v){
            case CommLevel::Manual:
                return "Manual";
            case CommLevel::Process:
                return "Process";
            case CommLevel::Localhost:
                return "Localhost";
            case CommLevel::LocalNetwork:
                return "LocalNetwork";
        }
        return "Default";
    }
    static std::string type(){
        return "Enum<CommLevel>";
    }
    static std::string info(){
        return "Manual|Process|Localhost|LocalNetwork";
    }
};
} // namespace tbk::param
struct ProcessInfo{
    std::string uuid = "";
    int pid = 0;
    std::string name = "";
    std::string ns = "";
    std::string ip = "";
    int port = 0;
    std::string key = "";
    ProcessInfo() = default;
    ProcessInfo(const std::string& uuid, const int pid, const std::string& name,const std::string& ns,const std::string address, const int port):uuid(uuid),pid(pid),name(name),ns(ns),ip(address),port(port),key(fmt::format("PROC:{}({}:{})",name,uuid,pid)){}
};
struct SubscriberInfo{
    std::string ip = "";
    int port = 0;
    std::string puuid = "";
    std::string uuid = "";
    int pid = 0;
    std::string ns = "default";
    std::string name = "";
    std::string msg_name = "";
    std::string node_name = "";
    std::string node_ns = "";
    void* ptr = nullptr;
    InfoFrom origin = InfoFrom::UNASSIGNED;
    CommLevel commLevel = CommLevel::Default;
    SubscriberInfo() = default;
    SubscriberInfo(const std::string& ip, const int port, const std::string& puuid, const int pid,const std::string& node_name,const std::string& node_ns,const std::string& ns,const std::string& uuid, const std::string& name, const std::string& msg_name,void* ptr = nullptr,const InfoFrom origin=InfoFrom::UNASSIGNED):ip(ip),port(port),puuid(puuid),pid(pid),node_name(node_name),node_ns(node_ns),ns(ns),uuid(uuid),name(name),msg_name(msg_name),ptr(ptr),origin(origin){}
    bool operator<(const SubscriberInfo& rhs) const{
        return uuid < rhs.uuid;
    }
};
struct PublisherInfo{
    std::string ip = "";
    std::string puuid = "";
    std::string uuid = "";
    int pid = 0;
    std::string ns = "default";
    std::string name = "";
    std::string msg_name = "";
    std::string node_name = "";
    std::string node_ns = "";
    void* ptr = nullptr;
    InfoFrom origin = InfoFrom::UNASSIGNED;
    CommLevel commLevel = CommLevel::Default;
    std::set<SubscriberInfo> subs;
    PublisherInfo() = default;
    PublisherInfo(const std::string& ip, const std::string& puuid,const int pid,const std::string& node_name,const std::string& node_ns,const std::string& ns, const std::string& uuid,const std::string& name, const std::string& msg_name,void* const ptr = nullptr,const InfoFrom origin=InfoFrom::UNASSIGNED):ip(ip),puuid(puuid),pid(pid),node_name(node_name),node_ns(node_ns),ns(ns),uuid(uuid),name(name),msg_name(msg_name),subs({}),ptr(ptr),origin(origin){}
    PublisherInfo(const std::string& ip, const std::string& puuid,const int pid,const std::string& node_name,const std::string& node_ns,const std::string& ns, const std::string& uuid,const std::string& name, const std::string& msg_name,const SubscriberInfo subs):ip(ip),puuid(puuid),pid(pid),node_name(node_name),node_ns(node_ns),ns(ns),uuid(uuid),name(name),msg_name(msg_name),subs({subs}){}
    bool addSubs(const SubscriberInfo& subs_info){
        subs.insert(subs_info);
        return true;
    }
    bool removeSubs(const SubscriberInfo& subs_info){
        subs.erase(subs_info);
        return true;
    }
    bool hasSub(const SubscriberInfo& sub_info) const{
        return subs.find(sub_info) != subs.end();
    }
};
struct HandleResult{
    bool success = false;
    std::string msg = "";
    HandleResult() = default;
    HandleResult(bool success, const std::string& msg=""):success(success),msg(msg){}
}; // TODO use ReqResult instead
template<typename TPub, typename TSub=TPub>
struct Info{
    std::map<TPub,PublisherInfo> pubs;
    std::map<TSub,SubscriberInfo> subs;
    HandleResult addPub(TPub ptr,const PublisherInfo& pub){
        return updatePub(ptr,pub);
    }
    HandleResult addSub(TSub ptr,const SubscriberInfo& sub){
        return updateSub(ptr,sub);
    }
    HandleResult removePub(TPub ptr,const PublisherInfo& pub){
        if(pubs.find(ptr) == pubs.end()){
            return {false,"publisher not found"};
        }
        pubs.erase(ptr);
        return {true,"success"};
    }
    HandleResult removeSub(TSub ptr,const SubscriberInfo& sub){
        if(subs.find(ptr) == subs.end()){
            return {false,"subscriber not found"};
        }
        subs.erase(ptr);
        return {true,"success"};
    }
    HandleResult updatePub(TPub ptr,const PublisherInfo pub){
        auto it = pubs.find(ptr);
        if(it != pubs.end()){
            pubs.erase(it);
        }
        pubs.insert({ptr,pub});
        return {true,"success"};
    }
    HandleResult updateSub(TSub ptr,const SubscriberInfo sub){
        auto it = subs.find(ptr);
        if(it != subs.end()){
            subs.erase(it);
        }
        subs.insert({ptr,sub});
        return {true,"success"};
    }
    bool checkSub(TSub ptr){
        return subs.find(ptr) != subs.end();
    }
    bool checkPub(TPub ptr){
        return pubs.find(ptr) != pubs.end();
    }
    SubscriberInfo& getSub(TSub ptr){
        return subs[ptr];
    }
    PublisherInfo& getPub(TPub ptr){
        return pubs[ptr];
    }
};
class PublisherBase;
class SubscriberBase;
class InfoHandler{
public:
    InfoHandler() = default;
    ~InfoHandler() = default;
    HandleResult addPub(PublisherBase* ptr,const PublisherInfo& pub);
    HandleResult addSub(SubscriberBase* ptr,const SubscriberInfo& sub);
    HandleResult removePub(PublisherBase* ptr,const PublisherInfo& pub);
    HandleResult removeSub(SubscriberBase* ptr,const SubscriberInfo& sub);
    HandleResult updatePub(PublisherBase* ptr,const PublisherInfo& pub);
    HandleResult updateSub(SubscriberBase* ptr,const SubscriberInfo& sub);
    HandleResult addOuter(const ProcessInfo&);
    HandleResult removeOuter(const ProcessInfo&);
    // HandleResult addOuterPub(const PublisherInfo& pub);
    // HandleResult addOuterSub(const SubscriberInfo& sub);
    HandleResult removeOuterPub(const PublisherInfo& pub);
    HandleResult removeOuterSub(const SubscriberInfo& sub);
    HandleResult updateOuterPub(const PublisherInfo& pub);
    HandleResult updateOuterSub(const SubscriberInfo& sub);
protected:
    Info<tbk::PublisherBase*,tbk::SubscriberBase*> _self_info;
    std::map<std::string,std::pair<std::time_t,Info<std::string>>> _outer_infos;
    std::map<std::string,ProcessInfo> _process_infos;
    std::mutex _self_info_mutex;
    std::mutex _outer_infos_mutex;
};
namespace type{
    using socket_callback_type = std::function<void(const void*,size_t)>;
    using socket_ep_callback_type = std::function<void(const boost::asio::ip::udp::endpoint&,const void*,size_t)>;
    using callback_type = std::function<void(const tbk::Data&)>;
    using param_callback_type = std::function<void(const std::string&,const std::string&)>;
} // namespace tbk::type
} // namespace tbk
#endif // __TBK_META_TYPES_H__
