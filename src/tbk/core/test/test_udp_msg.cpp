#undef TBK_DEBUG
#include "tbk/tbk.h"
void callback(const tbk::Data& data){
    std::string str(static_cast<const char*>(data.data()),data.size());
    tbk::log(fmt::fg(fmt::color::dodger_blue),"!!!!!!!callback1 : data_size={}, s='{}'\n",data.size(),str);
}
void callback2(const tbk::Data& data){
    std::string str(static_cast<const char*>(data.data()),data.size());
    tbk::log(fmt::fg(fmt::color::dodger_blue),"!!!!!!callback2 : data_size={}, s='{}'\n",data.size(),str);
}

int main(int argc, char *argv[]){
    std::string symbol = "";
    if (argc>1) symbol = argv[1]; 
    tbk::init("test_udp_msg",symbol);
    tbk::warning("test_udp_msg start : argc : {}\n",argc);
    if (argc < 2){
        // only monitor
        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }else if (argc < 3){
        tbk::Subscriber<3> s("test/suber","test_msg",callback);
        while(true){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }else{
        auto sleep_func = [](){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
                tbk::Subscriber<3> sub2(fmt::format("test/suber(2){}",symb),"test_msg_not",callback2);
                p.link(&sub2);
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
    }
    return 0;
}
