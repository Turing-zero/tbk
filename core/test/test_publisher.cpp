#include <iostream>
#include "tbk/core.h"

int main(){
    tbk::log("hello tbk!\n");
    tbk::Subscriber<3> s("test");
    tbk::Publisher p("test");
    p.publish(nullptr,0);
    std::thread t1([&]{
        tbk::Data data;
        while(true){
            auto res = s.try_get(data);
            std::string s(static_cast<const char*>(data.data()),data.size());
            tbk::info("res : {}. data_size={}, s={}\n",res,data.size(),s);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });
    std::thread t2([&]{
        int count = 0;
        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            std::string s = fmt::format("format string:{}",count);
            tbk::status("publish : {}\n",count);
            p.publish(s.c_str(),s.size());
            tbk::status("after pub: {}\n",count++);
            if(count > 8) {
                p.publish(nullptr,0);
                break;
            }
        }
    });
    t1.join();
    t2.join();
    return 0;
}