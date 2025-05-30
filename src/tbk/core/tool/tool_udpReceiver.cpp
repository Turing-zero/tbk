#include "tbk/tbk.h"
#include "tbk/log.h"
struct MsgInfo{
    std::string ip = "";
    int port = 0;
    std::string msg = "";
    MsgInfo(const std::string& ip, int port, const std::string& msg):ip(ip),port(port),msg(msg){}
    MsgInfo(){}
};

int main(int argc, char *argv[]){
    if (argc != 3 && argc != 4){
        tbk::error("usage : tool_udpSender [ip] <port> <msg>\n");
        return -1;
    }
    MsgInfo msgInfo;
    if(argc == 3){
        msgInfo = MsgInfo("0.0.0.0",std::stoi(argv[1]),argv[2]);
    }else{
        msgInfo = MsgInfo(argv[1],std::stoi(argv[2]),argv[3]);
    }
    tbk::init(fmt::format("udpReceiver_{}({}:{})",msgInfo.msg,msgInfo.ip,msgInfo.port));
    tbk::Publisher p("puber4udpreceive",msgInfo.msg);
    tbk::Data data;

    auto cb = [&](const void* ptr,size_t lens){
        p.publish(ptr,lens);
    };

    tbk::udp::socket socket;
    tbk::udp::endpoint ep(tbk::udp::address::from_string(msgInfo.ip),msgInfo.port);
    socket.bind(ep,cb);
    return 0;
}