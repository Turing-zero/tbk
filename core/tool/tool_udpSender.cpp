#include "tbk/tbk.h"
struct MsgInfo{
    std::string ip = "";
    int port = 0;
    std::string msg = "";
    MsgInfo(const std::string& ip, int port, const std::string& msg):ip(ip),port(port),msg(msg){}
    MsgInfo(){}
};
int main(int argc, char *argv[]){
    if (argc != 3 && argc != 4){
        tbk::error("usage : tbk::udpSender [ip] <port> <msg>\n");
        return -1;
    }
    MsgInfo msgInfo;
    if(argc == 3){
        msgInfo = MsgInfo("0.0.0.0",std::stoi(argv[1]),argv[2]);
    }else{
        msgInfo = MsgInfo(argv[1],std::stoi(argv[2]),argv[3]);
    }
    tbk::init(fmt::format("udpSender_{}({}:{})",msgInfo.msg,msgInfo.ip,msgInfo.port));
    tbk::Subscriber<3> s("suber4udpsend",msgInfo.msg);
    s.setCommLevel(tbk::CommLevel::Localhost);
    tbk::udp::socket socket;
    tbk::udp::endpoint ep(tbk::udp::address::from_string(msgInfo.ip),msgInfo.port);
    tbk::Data data;
    while(true){
        s.get(data);
        socket.send_to(data.data(),data.size(),ep);
    }
    return 0;
}