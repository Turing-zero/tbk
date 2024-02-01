#include "tbk/log.h"
#include "tbk/socketplugin.h"
#include "tbk/protocol/tbk_test.pb.h"
void _cb(const tbk::pb::Msg4Test& msg){
    std::cout << "Received: " << msg.DebugString() << std::endl;
}
int main(int argc, char *argv[]){
    std::string symbol = "";
    if (argc==2 && (argv[1] == "pub" or argv[1] == "sub")){
        symbol = argv[1];  
    }else{
        std::cout << "Usage: " << argv[0] << " pub|sub" << std::endl;
        return -1;
    } 
    if (symbol == "sub"){
        tbk::udp::Plugin<tbk::pb::Msg4Test> plugin(12321,_cb);
        while (true){
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }else{
        tbk::udp::Plugin<tbk::pb::Msg4Test> plugin;
        tbk::pb::Msg4Test data;
        int count = 0;
        while (true){
            data.set_name(fmt::format("testmsg-{}",count++));
            plugin.send(data,12321);
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}