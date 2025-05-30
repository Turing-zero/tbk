#include "tbk/tbk.h"
#include <chrono>
using namespace std::chrono_literals;

tbk::Semaphore s(1);
void pong(){
    static int count=0;
    // while(!_t.stop_requested()){
    while(true){
        if(s.try_acquire_for(1000ms)){
            tbk::log("get {} from pong\n",count++);
            std::this_thread::sleep_for(500ms);
        }else{
            tbk::log("wait for 1 second... timeout!\n");
            break;
        }
    }
    tbk::log("pong stopped\n");
}

int main(int argc, char *argv[])
{
    std::thread t(pong);
    for(int i=0;i<6;i++){
        std::this_thread::sleep_for(300ms);
        tbk::log("send {} from main\n",i);
        s.release();
    }
    t.join();
    return 0;
}
