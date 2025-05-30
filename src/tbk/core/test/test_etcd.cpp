#include "etcd/KeepAlive.hpp"
#include "tbk/process/etcdif.h"
static const std::string url = "http://127.0.0.1:2379";
static const std::string etcd_url = etcdv3::detail::resolve_etcd_endpoints("http://127.0.0.1:2379");
void cb(::etcd::Response const& resp){
    std::cout << "abaaba" << std::endl;
    if (resp.error_code()) {
        std::cout << resp.error_code() << ": " << resp.error_message() << std::endl;
        return;
    }
    std::cout << resp.action() << " " << resp.value().as_string() << std::endl;
    std::cout << "Previous value: " << resp.prev_value().as_string() << std::endl;

    std::cout << "Events size: " << resp.events().size() << std::endl;
    for (auto const &ev: resp.events()) {
        if (ev.prev_kv().key().find("/leader") == 0 || ev.kv().key().find("/leader") == 0) {
            return;
        }
        std::cout << "Value change in events: " << static_cast<int>(ev.event_type())
        << ", prev kv = " << ev.prev_kv().key() << " -> " << ev.prev_kv().as_string()
        << ", kv = " << ev.kv().key() << " -> " << ev.kv().as_string()
        << std::endl;
    }
}
void printResponse(::etcd::Response const& resp){
    std::cout << "-----------"<< std::endl;
    if (resp.error_code()) {
        std::cout << "err : " << resp.error_code() << ": " << resp.error_message() << std::endl;
        std::cout << "-----------\n"<< std::endl;
        return;
    }
    std::cout << resp.action() << " " << resp.value().as_string() << std::endl;
    std::cout << "Keys size: " << resp.keys().size() << std::endl;
    for(auto const& kv: resp.keys()){
        std::cout << "key = " << kv << std::endl;
    }
    std::cout << "Values size: " << resp.values().size() << std::endl;
    for(auto const& kv: resp.values()){
        std::cout << "value = " << kv.as_string() << std::endl;
    }
    std::cout << "Events size: " << resp.events().size() << std::endl;
    for (auto const &ev: resp.events()) {
        std::cout << "Value change in events: " << static_cast<int>(ev.event_type())
        << ", prev kv = " << ev.prev_kv().key() << " -> " << ev.prev_kv().as_string()
        << ", kv = " << ev.kv().key() << " -> " << ev.kv().as_string()
        << std::endl;
    }
    std::cout << "-----------\n"<< std::endl;
}
int main(int argc, char** argv){
    std::string prefix="/test";
    tbk::InfoHandler infoHandler;
    tbk::etcd::ClientParam param{infoHandler};
    tbk::etcd::Client client(param);
    ::etcd::SyncClient etcd(etcd_url);
    auto keepalive = etcd.leasekeepalive(3);
    auto lease_id = keepalive->Lease();
    // etcd.rmdir(prefix, true);
    printResponse( etcd.add(prefix+"/key1", "42",lease_id) );
    printResponse( etcd.get(prefix+"/key1") );
    printResponse( etcd.get(prefix+"/key2") );
    printResponse( etcd.ls(prefix) );
    printResponse( etcd.keys(prefix) );
    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return 0;
}