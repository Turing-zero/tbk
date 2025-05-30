#include <iostream>
#include "tbk/tbk.h"

int main(){
    tbk::init("test_pong_cpp");

    int count = 0;
    tbk::Publisher p("pong_p","pong");
    auto callback = [&](const tbk::Data& data){
        count++;
        p.publish("pong");
    };
    tbk::Subscriber<3> s("ping_s","ping",callback);
    while(true){
        if (count > 10000) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}