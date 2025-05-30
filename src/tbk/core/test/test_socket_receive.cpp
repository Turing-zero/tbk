#include <iostream>
#include <string>
#include <chrono>
#include "tbk/socket.h"
void _cb(const void* p,size_t lens){
    std::string s(static_cast<const char*>(p),lens);
    std::cout << "marktest : " << s << std::endl;
}

void test(){
    tbk::udp::socket socket;
    auto res = socket.bind_any(_cb);
    std::cout << "bind ep : " << socket.get_bind_ep() << std::endl;
}
int main(){
    test();
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}
