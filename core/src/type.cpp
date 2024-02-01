#include <list>
#include "tbk/type.h"
#include "tbk/core.h"
namespace tbk{
HandleResult InfoHandler::addPub(PublisherBase* ptr,const PublisherInfo& pub){
    std::unique_lock lock{_self_info_mutex};
    if(getenv("TBK_DEBUG_INFO")){
        tbk::log("info : addPub-{}-{}\n",pub.msg_name,fmt::ptr(ptr));
    }
    auto res = _self_info.addPub(ptr,pub);
    if(!res.success){
        return res;
    }
    auto& rPub = _self_info.getPub(ptr);
    // iterate self subs and check if any match
    for(auto& [sub_ptr,sub]:_self_info.subs){
        if(pub.msg_name == sub.msg_name){
            rPub.addSubs(sub);
            lock.unlock();
            static_cast<tbk::PublisherBase*>(pub.ptr)->link((sub_ptr));
            lock.lock();
        }
    }
    std::set<void*> updateList = {};
    // iterate outer subs and check if any match
    for(auto& [puuid,pair]:_outer_infos){
        for(auto& [uuid,sub]:pair.second.subs){
            if(pub.msg_name == sub.msg_name && pub.node_ns == sub.node_ns && !rPub.hasSub(sub)){
                if(!sub.ip.empty() && sub.port != 0){
                    rPub.addSubs(sub);
                    static_cast<tbk::PublisherBase*>(pub.ptr)->link_u(false,sub);
                    updateList.insert(pub.ptr);
                }
            }
        }
    }
//    lock.unlock();
//    for(auto& ptr:updateList){
//        static_cast<tbk::PublisherBase*>(ptr)->update2manager();
//    }
    return res;
}
HandleResult InfoHandler::addSub(SubscriberBase* ptr,const SubscriberInfo& sub){
    std::unique_lock lock{_self_info_mutex};
    if(getenv("TBK_DEBUG_INFO")){
        tbk::log("info : addSub-{}-{}\n",sub.msg_name,fmt::ptr(ptr));
    }
    auto res = _self_info.addSub(ptr,sub);
    if(!res.success){
        return res;
    }
    std::set<void*> updateList = {};
    // iterate self pubs and check if any match
    for(auto& [ptr,pub]:_self_info.pubs){
        if(pub.msg_name == sub.msg_name){
            pub.addSubs(sub);
            static_cast<tbk::PublisherBase*>(pub.ptr)->link(false,static_cast<tbk::SubscriberBase*>(sub.ptr));
            updateList.insert(pub.ptr);
        }
    }
//    lock.unlock();
//    for(auto& ptr:updateList){
//        static_cast<tbk::PublisherBase*>(ptr)->update2manager();
//    }
    return res;
}
HandleResult InfoHandler::removePub(PublisherBase* ptr,const PublisherInfo& pub){
    std::unique_lock lock{_self_info_mutex};
    if(getenv("TBK_DEBUG_INFO")){
        tbk::log("info : removePub-{}-{}\n",pub.msg_name,fmt::ptr(ptr));
    }
    // check if exist
    if(!_self_info.checkPub(ptr)){
        return {false,"publisher not found"};
    }
    auto& rPub = _self_info.getPub(ptr);
    // iterate self subs and check if any match
    for(auto& [ptr,sub]:_self_info.subs){
        if(rPub.hasSub(sub)){
            rPub.removeSubs(sub);
            static_cast<tbk::PublisherBase*>(pub.ptr)->unlink(false,static_cast<tbk::SubscriberBase*>(sub.ptr));
        }
    }
    return _self_info.removePub(ptr,pub);
}
HandleResult InfoHandler::removeSub(SubscriberBase* ptr,const SubscriberInfo& sub){
    std::unique_lock lock{_self_info_mutex};
    if(getenv("TBK_DEBUG_INFO")){
        tbk::log("info : removeSub-{}-{}\n",sub.msg_name,fmt::ptr(ptr));
    }
    // check if exist
    if(!_self_info.checkSub(ptr)){
        return {false,"subscriber not found"};
    }
    // iterate self pubs and check if any match
    std::set<void*> updateList = {};
    for(auto & [ptr,pub]:_self_info.pubs){
        if(pub.hasSub(sub)){
            pub.removeSubs(sub);
            static_cast<tbk::PublisherBase*>(pub.ptr)->unlink(false,static_cast<tbk::SubscriberBase*>(sub.ptr));
            updateList.insert(pub.ptr);
        }
    }
    auto res = _self_info.removeSub(ptr,sub);
//    lock.unlock();
//    for(auto& ptr:updateList){
//        static_cast<tbk::PublisherBase*>(ptr)->update2manager();
//    }
    return res;
}
HandleResult InfoHandler::updatePub(PublisherBase* ptr,const PublisherInfo& pub){
    std::unique_lock lock{_self_info_mutex};
    if(getenv("TBK_DEBUG_INFO")){
        tbk::log("info : updatePub-{}-{}\n",pub.msg_name,fmt::ptr(ptr));
    }
    auto res = _self_info.updatePub(ptr,pub);
    return res;
}
HandleResult InfoHandler::updateSub(SubscriberBase* ptr,const SubscriberInfo& sub){
    std::unique_lock lock{_self_info_mutex};
    if(getenv("TBK_DEBUG_INFO")){
        tbk::log("info : updateSub-{}-{}\n",sub.msg_name,fmt::ptr(ptr));
    }
    // can only use to change ip and port
    auto res = _self_info.updateSub(ptr,sub);
    return res;
}
HandleResult InfoHandler::addOuter(const ProcessInfo& info){
    std::unique_lock lock{_self_info_mutex};
    auto uuid = info.uuid;
    auto it = _outer_infos.find(uuid);
    if(it != _outer_infos.end()){
        return {false,"uuid already exist"};
    }
    _outer_infos[uuid] = std::make_pair(std::time(nullptr),Info<std::string>());
    _process_infos[uuid] = info;
    return {true,"success"};
}
HandleResult InfoHandler::removeOuter(const ProcessInfo& info){
    std::unique_lock lock{_self_info_mutex};
    auto uuid = info.uuid;
    auto it = _outer_infos.find(uuid);
    if(it == _outer_infos.end()){
        return {true,"uuid not exist"};
    }
    std::set<void*> updateList = {};
    // iterate outer subs & self pubs and check if any match
    for(auto& [uuid,sub]:it->second.second.subs){
        for(auto& [ptr,pub]:_self_info.pubs){
            if(pub.msg_name == sub.msg_name){
                pub.removeSubs(sub);
                static_cast<tbk::PublisherBase*>(pub.ptr)->unlink_u(false,sub);
                updateList.insert(pub.ptr);
            }
        }
    }
    _outer_infos.erase(it);
    _process_infos.erase(uuid);
//    lock.unlock();
//    for(auto& ptr:updateList){
//        static_cast<tbk::PublisherBase*>(ptr)->update2manager();
//    }
    return {true,"success"};
}
HandleResult InfoHandler::removeOuterPub(const PublisherInfo& pub){
    std::unique_lock lock{_self_info_mutex};
    auto it = _outer_infos.find(pub.puuid);
    if(it == _outer_infos.end()){
        return {true,"uuid not exist"};
    }
    auto res = it->second.second.removePub(pub.name,pub);
    if(res.success){
        it->second.first = std::time(nullptr);
    }
    return res;
}
HandleResult InfoHandler::removeOuterSub(const SubscriberInfo& sub){
    std::unique_lock lock{_self_info_mutex};
    auto it = _outer_infos.find(sub.puuid);
    if(it == _outer_infos.end()){
        return {true,"uuid not exist"};
    }
    auto res = it->second.second.removeSub(sub.name,sub);
    if(!res.success){
        return res;
    }
    it->second.first = std::time(nullptr);
    std::set<void*> updateList = {};
    // iterate self pubs and check if any match
    for(auto& [ptr,pub]:_self_info.pubs){
        if(pub.msg_name == sub.msg_name && pub.hasSub(sub)){
            pub.removeSubs(sub);
            static_cast<tbk::PublisherBase*>(pub.ptr)->unlink_u(false,sub);
            updateList.insert(pub.ptr);
        }
    }
//    lock.unlock();
//    for(auto& ptr:updateList){
//        static_cast<tbk::PublisherBase*>(ptr)->update2manager();
//    }
    return res;
}
HandleResult InfoHandler::updateOuterPub(const PublisherInfo& pub){
    std::unique_lock lock{_self_info_mutex};
    auto it = _outer_infos.find(pub.puuid);
    if(it == _outer_infos.end()){
        return {false,"uuid not exist"};
    }
    auto res = it->second.second.updatePub(pub.name,pub);
    if(res.success){
        it->second.first = std::time(nullptr);
    }
    return res;
}
HandleResult InfoHandler::updateOuterSub(const SubscriberInfo& sub){
    std::unique_lock lock{_self_info_mutex};
    auto it = _outer_infos.find(sub.puuid);
    if(it == _outer_infos.end()){
        return {false,"uuid not exist"};
    }
    auto res = it->second.second.updateSub(sub.name,sub);
    if(res.success){
        it->second.first = std::time(nullptr);
    }
    std::set<void*> updateList = {};
    // iterate self pubs and check if any match
    for(auto& [ptr,pub]:_self_info.pubs){
        if(pub.msg_name == sub.msg_name && !pub.hasSub(sub)){
            if(!sub.ip.empty() && sub.port != 0){
                pub.addSubs(sub);
                static_cast<tbk::PublisherBase*>(pub.ptr)->link_u(false,sub);
                updateList.insert(pub.ptr);
            }
        }
    }
//    lock.unlock();
//    for(auto& ptr:updateList){
//        static_cast<tbk::PublisherBase*>(ptr)->update2manager();
//    }
    return res;
}
} // namespace tbk
