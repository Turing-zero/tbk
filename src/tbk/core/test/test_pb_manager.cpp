#include <iostream>
#include "tbk/tbk.h"
int main(){
    tbk::init("test_pb_manager");
    tbk::Subscriber<3> s("test_suber");
    tbk::Publisher p("test_puber");
    p.publish(nullptr,0);
    std::thread t1([&]{
        tbk::Data data;
        while(true){
            auto res = s.try_get(data);
            std::string str(static_cast<const char*>(data.data()),data.size());
            tbk::log(fmt::fg(fmt::color::cyan),"res : {}. data_size={}, s='{}'\n",res,data.size(),str);
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        }
    });
    std::thread t2([&]{
        int count = 0;
        while(true){
            std::string s = fmt::format("format string:{}",count);
            tbk::log(fmt::fg(fmt::color::cyan),"publish : {}\n",count++);
            p.publish(s.c_str(),s.size());
            std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            if(count > 6) break;
        }
    });
    // p.link(&s);
    t1.join();
    t2.join();
    tbk::info("end of main func\n");
    return 0;
}
