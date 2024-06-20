#include "tbk/utils/circlequeue.h"
#include "tbk/core_monitor.h"
#include <thread>
#include <fmt/format.h>
#include <fmt/ranges.h>
int main(){
    {
        tbk::DataQueue<int, 10> dq;
        for(int i=0;i<10;i++){
            dq.push(i);
        }
        for(int i=-9;i<=0;i++){
            fmt::print("{} : {}\n", i, dq[i]);
        }
        for(int i=0;i<10;i++){
            dq.push(i+10);
        }
        for(int i=-9;i<=0;i++){
            fmt::print("{} : {}\n", i, dq[i]);
        }
    }
    fmt::print("========================================\n");
    {
        tbk::MsgWrapMonitor monitor;
        monitor.addMsg("uuid1", {
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now(),
            0
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        for(int i=1;i<100;i++){
            monitor.addMsg("uuid1", {
                std::chrono::system_clock::now(),
                std::chrono::system_clock::now(),
                i+1
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
        auto freqRes = monitor.getFreq("uuid1");
        fmt::print("freqRes: {}\n", freqRes);
        freqRes = monitor.getFreq("uuid2");
        fmt::print("freqRes: {}\n", freqRes);
        auto hitRateRes = monitor.getHitRate("uuid1");
        fmt::print("hitRateRes: {}\n", hitRateRes);
        hitRateRes = monitor.getHitRate("uuid2");
        fmt::print("hitRateRes: {}\n", hitRateRes);
    }
    return 0;
}