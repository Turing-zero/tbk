#include <iostream>
#include <string>
#include "tbk/socket.h"
#include "tbk/socketplugin.h"
#include "tzcp/tbk/tbk_test.pb.h"
void _cb(const void* p,size_t lens){
    std::string s(static_cast<const char*>(p),lens);
    std::cout << "marktest : " << s << std::endl;
}

void test(){
    int port = 30001;
    boost::asio::ip::udp::endpoint receiver_endpoint(tbk::udp::address_v4::any(),port);

    tbk::udp::socket socket;
    socket.join_multicast(tbk::udp::address::from_string("233.233.233.233"),tbk::udp::address::from_string("10.12.225.58"));
    socket.bind(receiver_endpoint,_cb);

    tbk::__io::_()->run();
}
int main(){
    test();
    return 0;
}
