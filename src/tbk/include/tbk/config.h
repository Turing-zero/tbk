#ifndef __TBK_CONFIG_H__
#define __TBK_CONFIG_H__
#include <chrono>
namespace tbk{
namespace config{
    using namespace std::chrono_literals;
    constexpr auto timestep = 50ms;
    constexpr int threadpool_nums = 4;
}// tbk::config
constexpr auto __timestep = config::timestep;
}// tbk
#endif // __TBK_CONFIG_H__
