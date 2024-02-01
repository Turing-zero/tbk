#ifndef __TBK_UTILS_RATE_H__
#define __TBK_UTILS_RATE_H__
#include <chrono>
#include <thread>
namespace tbk{
class Rate{
public:
    Rate(const int frq){
        _last_t = std::chrono::steady_clock::now();
        _step = std::chrono::nanoseconds(1000000000)/frq;
    }
    void sleep(){
        std::this_thread::sleep_until(_last_t+_step);
        _last_t = _last_t+_step;
    }
    void reset(){
        _last_t = std::chrono::steady_clock::now();
    }
private:
    std::chrono::time_point<std::chrono::steady_clock> _last_t;
    std::chrono::duration<int, std::nano> _step;
};
} // namespace tbk;
#endif // __TBK_UTILS_RATE_H__