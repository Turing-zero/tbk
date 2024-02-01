#include <iostream>
#include <chrono>
#include <thread>
#include <fmt/format.h>
#include "tbk/socket.h"

int main(){
    int count = 0;
    int port = 30001;
    tbk::udp::socket socket;
    socket.set_interface(tbk::udp::address::from_string("10.12.225.58"));

    boost::asio::ip::udp::endpoint receiver_endpoint(tbk::udp::address::from_string("233.233.233.233"),port);

    while(true){
        socket.send_to(fmt::format("count : {}",count++),receiver_endpoint);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
