#include <iostream>
#include <chrono>
#include "tbk/tbk.h"

int main(){
    tbk::init("test_ping_cpp");

    int count = 0;
    tbk::Publisher p("ping_p","ping");
    auto callback = [&](const tbk::Data& data){
        count++;
        p.publish("ping");
    };
    tbk::Subscriber<3> s("pong_s","pong",callback);
    std::cout << "Press Enter to continue..." << std::endl;
    std::cin.get();
    auto begin = std::chrono::steady_clock::now();
    p.publish("ping");
    while(true){
        if (count > 10000) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()/1000000.0 << "[s]" << std::endl;
    return 0;
}