#ifndef __TBKCORE_MONITOR_H__
#define __TBKCORE_MONITOR_H__
#include <mutex>
#include <chrono>
#include <map>
#include <tuple>
#include "tbk/utils/circlequeue.h"
namespace tbk{
template<typename T>
using ReqResult = std::tuple<bool, T, std::string>;
struct MsgWrapInfo{
    std::chrono::time_point<std::chrono::system_clock> recv_time;
    std::chrono::time_point<std::chrono::system_clock> sent_time;
    int recv_index = -1;
};
class MsgWrapMonitor{
    static const int CALC_MIN_PACK_COUNT = 3;
    static const int CALC_MAX_PACK_COUNT = 500;
    static const int CALC_MIN_TIME = 50; // ms
    static const int CALC_MAX_TIME = 1000; // ms
public:
    ReqResult<float> getFreq();
    ReqResult<float> getFreq(const std::string& name);
    ReqResult<float> getHitRate(const std::string& name);
    void addMsg(const std::string& pub_uuid, const MsgWrapInfo& info);
private:
    std::mutex _mutex;
    std::map<std::string,tbk::DataQueue<MsgWrapInfo, CALC_MAX_PACK_COUNT>> _recv_times;
};
} // namespace tbk
#endif // __TBKCORE_MONITOR_H__