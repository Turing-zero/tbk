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
    std::chrono::microseconds recv_time;
    std::chrono::microseconds sent_time;
    int recv_index = -1;
};
class MsgWrapMonitor{
    static constexpr int CALC_MIN_PACK_COUNT = 3;
    static constexpr int CALC_MAX_PACK_COUNT = 500;
    static constexpr std::chrono::milliseconds CALC_MIN_TIME = std::chrono::milliseconds(50);
    static constexpr std::chrono::milliseconds CALC_MAX_TIME = std::chrono::milliseconds(1000);
public:
    ReqResult<float> getFreq();
    ReqResult<float> getFreq(const std::string& name);
    ReqResult<float> getHitRate(const std::string& name);
    void addMsg(const std::string& pub_uuid, const MsgWrapInfo& info);
private:
    std::mutex _mutex;
    std::map<std::string,tbk::CircleQueue<MsgWrapInfo, CALC_MAX_PACK_COUNT>> _recv_times;
};
} // namespace tbk
#endif // __TBKCORE_MONITOR_H__