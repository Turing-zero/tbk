#undef TBK_DEBUG
#include "tbk/tbk.h"
#include <iostream>
void callback(const tbk::Data& data){
    std::string str(static_cast<const char*>(data.data()),data.size());
    tbk::log(fmt::fg(fmt::color::dodger_blue),"!!!!!!!callback1 : data_size={}, s='{}'\n",data.size(),str);
}
void callback2(const tbk::Data& data){
    std::string str(static_cast<const char*>(data.data()),data.size());
    tbk::log(fmt::fg(fmt::color::dodger_blue),"!!!!!!callback2 : data_size={}, s='{}'\n",data.size(),str);
}
int main(int argc, char *argv[]){
    std::cout << "in main" << std::endl;
    tbk::init("test_udp_msg","_symbol");
    std::cout << "in main2" << std::endl;
    tbk::warning("test_udp_msg start : argc : {}\n",argc);

    auto sleep_func = [](){
        // std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return;
    };
    auto run_func = [sleep_func](const std::string& symb){
        tbk::Publisher p(fmt::format("test/puber{}",symb),"test_msg");
        int count = 0;
        while(true){
            for(auto i=0;i<2;i++){
                sleep_func();
                std::string s = fmt::format("pub{} : {}",symb,count++);
                tbk::log(fmt::fg(fmt::color::cyan),s+"\n");
                p.publish(s.c_str(),s.size());
            }
            sleep_func();
            tbk::Subscriber<3> sub(fmt::format("test/suber{}",symb),"test_msg",callback);
            sub.setCommLevel(tbk::CommLevel::Localhost);
            for(auto i=0;i<2;i++){
                std::string s = fmt::format("pub(2){} : {}",symb,count++);
                tbk::log(fmt::fg(fmt::color::cyan),s+"\n");
                p.publish(s.c_str(),s.size());
                sleep_func();
            }
            std::string s = fmt::format("pub(3){} : {}",symb,count++);
            tbk::log(fmt::fg(fmt::color::cyan),s+"\n");
            p.publish(s.c_str(),s.size());
        }
    };
    // std::thread t1(run_func,"222");
    run_func("111");
    return 0;
}
