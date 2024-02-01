#include <iostream>
#include <chrono>
#include <thread>
#include <fmt/format.h>
#include "tbk/socket.h"

int main(int argc, char *argv[]){
    int count = 0;
    if(argc < 2){
        std::cout << "usage: " << argv[0] << "<port>" << std::endl;
        return 1;
    }
    int port = std::stoi(argv[1]);
    tbk::udp::socket socket;
    boost::asio::ip::udp::endpoint receiver_endpoint(boost::asio::ip::address::from_string("0.0.0.0"),port);

    while(true){
        socket.send_to(fmt::format("count : {}",count++),receiver_endpoint);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}
