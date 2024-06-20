#include "tbk/core_monitor.h"
#include <fmt/format.h>
#include <fmt/ranges.h>
namespace tbk{
template<int QUEUESIZE,typename CompareFunc>
// use to find the index of the first element that func return 0
static ReqResult<int> binarySearch(tbk::DataQueue<MsgWrapInfo, QUEUESIZE> dq, const CompareFunc& func, int left=1, int right=1){
    int size = dq.size();
    if (size < 1){
        return {false, 0, "size < 1"};
    }
    if(left > 0){
        left = -size+1;
    }
    if(right > 0){
        right = 0;
    }
    assert(left <= 0);
    assert(right <= 0);
    assert(left > -size);
    assert(right >= -size);
    assert(left <= right);
    int mid = 0;
    decltype(func(dq[0],0)) res;
    while (left <= right){
        mid = (left + right) / 2;
        res = func(dq[mid],mid);
        if (res == 0){
            break;
        }else if(res > 0){
            left = mid + 1;
        }else if(res < 0){
            right = mid - 1;
        }
    }
    if (res != 0){
        return {false, mid, fmt::format("not found. last res: {}", res)};
    }
    return {true, mid, ""};
}

ReqResult<float> MsgWrapMonitor::getFreq(){
    return {false, -1, "not implemented"};
}
ReqResult<float> MsgWrapMonitor::getFreq(const std::string& pub_uuid){
    if (_recv_times.find(pub_uuid) == _recv_times.end()){
        return {false, -1, "uuid not found"};
    }
    auto&& infos = _recv_times[pub_uuid];
    const auto NOW = std::chrono::system_clock::now();
    auto res = binarySearch<CALC_MAX_PACK_COUNT>(infos, [NOW](const MsgWrapInfo& info,const int index){
        int timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(NOW - info.recv_time).count();
        if (-index < CALC_MIN_PACK_COUNT){
            return -1;
        }else if (-index > CALC_MAX_PACK_COUNT){
            return 1;
        }
        if (timeDiff < CALC_MIN_TIME){
            return -1;
        }else if (timeDiff > CALC_MAX_TIME){
            return 1;
        }
        return 0;
    });
    if (!std::get<0>(res)){
        return {false, -1, std::get<2>(res)};
    }
    int index = std::get<1>(res);
    int timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(infos[0].recv_time - infos[index].recv_time).count();
    return {true, 1000.0*-index / timeDiff, fmt::format("{}pack/{}ms",-index,timeDiff)};
}
ReqResult<float> MsgWrapMonitor::getHitRate(const std::string& pub_uuid){
    if (_recv_times.find(pub_uuid) == _recv_times.end()){
        return {false, -1, "uuid not found"};
    }
    auto&& infos = _recv_times[pub_uuid];
    const int size = infos.size();
    if(size < 2){
        return {false, -1, "size < 2"};
    }
    int start = -(size-1);
    return {true, 1.0*-start/(infos[0].recv_index-infos[start].recv_index), fmt::format("{}pack/{}count",-start,infos[0].recv_index-infos[start].recv_index)};
}
void MsgWrapMonitor::addMsg(const std::string& pub_uuid, const MsgWrapInfo& info){
    if (_recv_times.find(pub_uuid) == _recv_times.end()){
        _recv_times[pub_uuid] = tbk::DataQueue<MsgWrapInfo, CALC_MAX_PACK_COUNT>();
    }
    _recv_times[pub_uuid].push(info);
}
} // namespace tbk