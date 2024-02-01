#include <iostream>
#include "tbk/process/etcdif.h"
#include "fmt/format.h"
static const std::string etcd_url = etcdv3::detail::resolve_etcd_endpoints("http://127.0.0.1:2379");
const std::string _prefix = "/tbk/ps";

void printSub(const tbk::SubscriberInfo& info,const std::string& _prefix=""){
    auto prefix = _prefix+"\t";
    std::string str = "";
    str += fmt::format("{}Sub :\t{}\n",_prefix,info.name);
    str += fmt::format("{}puuid :\t\t{}\n",prefix,info.puuid);
    str += fmt::format("{}uuid :\t\t{}\n",prefix,info.uuid);
    str += fmt::format("{}pid :\t\t{}\n",prefix,info.pid);
    str += fmt::format("{}namespace :\t{}\n",prefix,info.ns);
    str += fmt::format("{}name :\t\t{}\n",prefix,info.name);
    str += fmt::format("{}msg_name :\t{}\n",prefix,info.msg_name);
    str += fmt::format("{}node_name :\t{}\n",prefix,info.node_name);
    str += fmt::format("{}node_ns :\t{}\n",prefix,info.node_ns);
    str += fmt::format("{}ip :\t\t{}\n",prefix,info.ip);
    str += fmt::format("{}port :\t\t{}\n",prefix,info.port);
    fmt::print("{}",str);
}
void printPub(const tbk::PublisherInfo& info,const std::string& _prefix=""){
    auto prefix = _prefix+"\t";
    std::string str = "";
    str += fmt::format("{}Pub :\t{}\n",_prefix,info.name);
    str += fmt::format("{}puuid :\t\t{}\n",prefix,info.puuid);
    str += fmt::format("{}uuid :\t\t{}\n",prefix,info.uuid);
    str += fmt::format("{}pid :\t\t{}\n",prefix,info.pid);
    str += fmt::format("{}namespace :\t{}\n",prefix,info.ns);
    str += fmt::format("{}name :\t\t{}\n",prefix,info.name);
    str += fmt::format("{}msg_name :\t{}\n",prefix,info.msg_name);
    str += fmt::format("{}node_name :\t{}\n",prefix,info.node_name);
    str += fmt::format("{}node_ns :\t{}\n",prefix,info.node_ns);
    fmt::print("{}",str);
    for(auto const& sub : info.subs){
        printSub(sub,prefix);
    }
}
void printProcess(const std::string& prefix,const tbk::ProcessInfo& info){

}
void printResponse(::etcd::Response const& resp){
    std::cout << "-----------"<< std::endl;
    if (resp.error_code()) {
        std::cout << "err : " << resp.error_code() << ": " << resp.error_message() << std::endl;
        std::cout << "-----------\n"<< std::endl;
        return;
    }
    int index = -1;
    for(auto const& kv: resp.keys()){
        index++;
        auto keys = tbk::etcd::decodeKey(_prefix,kv);
        if(keys.size() == 1){
            std::cout << "ps : " << keys[0] << std::endl;
        }else if(keys.size() == 3){
            if(keys[1] == "pubs"){
                auto pub = tbk::etcd::decodePub(resp.value(index).as_string());
                printPub(pub);
            }else if(keys[1] == "subs"){
                auto sub = tbk::etcd::decodeSub(resp.value(index).as_string());
                printSub(sub);
            }else{
                std::cerr << fmt::format("unknown key:{}",kv) << std::endl;
            }
        }
    }
    // for(auto const& kv: resp.values()){
    //     std::cout << "value = " << kv.as_string() << std::endl;
    // }
    std::cout << "-----------\n"<< std::endl;
}

int main(){
    ::etcd::SyncClient etcd(etcd_url);
    printResponse( etcd.ls(_prefix) );
}