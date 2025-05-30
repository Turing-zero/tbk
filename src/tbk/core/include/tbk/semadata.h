#ifndef __TBK_SEMADATA_H__
#define __TBK_SEMADATA_H__
#include <cstring>
#include <thread>
#include <limits>
#include <mutex>
#include <shared_mutex>
#include <cassert>
#include "tbk/config.h"
#include "tbk/interface.h"
#include "tbk/log.h"

#include "tbk/data.h"

#include "tbk/semaphore.h"

namespace tbk{
using Semaphore = SemaphoreT;
class DataQueue:public IData{
public:
    DataQueue(unsigned int _max_capacity=100):_size(0),_max_capacity(_max_capacity),_capacity(1),_start(new DataNode()),_end(_start){
        assert(_max_capacity>0);
        #ifdef TBK_DEBUG
        tbk::log("{} TBK DataQueue constructor\n",fmt::ptr(this));
        #endif
        std::unique_lock<std::shared_mutex> lock(_mutex);
        _start->_last = _start;
        _start->_next = _start;
    }
    virtual ~DataQueue(){
        #ifdef TBK_DEBUG
        tbk::log("{} TBK DataQueue destructor\n",fmt::ptr(this));
        #endif
        while(_capacity>1){
            _start = _start->_next;
            delete _start->_last;
            _capacity--;
        }
        delete _start;
    }
    virtual void pop(Data& p) override{
        std::unique_lock<std::shared_mutex> lock(_mutex);
        p.store(_start->_data,_start->_size);
        _start = _start->_next;
        _size--;
        #ifdef TBK_DEBUG
            tbk::log("{} TBK DataQueue finish pop() ,size={},capa={}\n",fmt::ptr(this),_size,_capacity);
            if(_size < 0){
                tbk::log("{} size < 0 after pop({}). _size={}, _capacity={}\n",fmt::ptr(this),fmt::ptr(&p),_size,_capacity);
            }
        #endif
    }
    virtual void pop(Data* p=nullptr) override{
        std::unique_lock<std::shared_mutex> lock(_mutex);
        if(p!= nullptr){
            p->store(_start->_data,_start->_size);
        }
        _start = _start->_next;
        _size--;
        #ifdef TBK_DEBUG
            tbk::log("{} TBK DataQueue finish pop() ,size={},capa={}\n",fmt::ptr(this),_size,_capacity);
            if(_size < 0){
                tbk::log("{} size < 0 after pop({}). _size={}, _capacity={}\n",fmt::ptr(this),fmt::ptr(p),_size,_capacity);
            }
        #endif
    }
    virtual void store(const Data& data){
        store(data.data(),data._size);
    }
    virtual void store(const Data* data){
        store(data->data(),data->_size);
    }
    virtual void store(const void* const data,unsigned long size) override{
        std::unique_lock<std::shared_mutex> lock(_mutex);
        DataNode* storeNode = nullptr;
        if(_size == _max_capacity){
            storeNode = _end;
            _end = _end->_next;
            _start = _start->_next;
            _size--;
        }else if(_size < _capacity){
            storeNode = _end;
            _end = _end->_next;
        }else{
            storeNode = new DataNode(_end->_last,_end);
            _end->_last->_next = storeNode;
            _end->_last = storeNode;
            _capacity++;
        }
        storeNode->resize(size);
        if(size > 0)
            memcpy(storeNode->_data,data,size);
        _size++;
    }
    virtual unsigned long size(){
        std::shared_lock<std::shared_mutex> lock(_mutex);
        return _size;
    }
protected:
    unsigned int _size;
    unsigned int _capacity;
    unsigned int _max_capacity;
    DataNode* _start;
    DataNode* _end;
    mutable std::shared_mutex _mutex;
};

class SemaData{
public:
    SemaData(unsigned int _max_capacity=100):_semaphore(_max_capacity),_data(_max_capacity){
        #ifdef TBK_DEBUG
        tbk::log("{} TBK SemaData constructor. capa : {}\n",fmt::ptr(this),_max_capacity);
        #endif
    };
    virtual ~SemaData(){
        #ifdef TBK_DEBUG
        tbk::log("{} TBK SemaData destructor.\n",fmt::ptr(this));
        #endif
    };
//    SemaData(const SemaData&) = delete;
    virtual void pop(Data* p=nullptr){
        std::scoped_lock lock(_mutex);
        _semaphore.acquire();
        _data.pop(p);
    }
    virtual void pop(Data& p){
        std::scoped_lock lock(_mutex);
        _semaphore.acquire();
        _data.pop(p);
    }
    virtual void store(const void* const data,unsigned long size){
        std::scoped_lock lock(_mutex);
        _data.store(data,size);
        _semaphore.release();
    }
    virtual void release(){
        std::scoped_lock lock(_mutex);
        _semaphore.release();
    }
    virtual void acquire(){
        std::scoped_lock lock(_mutex);
        _semaphore.acquire();
    }
    virtual bool try_acquire(){
        std::scoped_lock lock(_mutex);
        return _semaphore.try_acquire();
    }
    virtual bool try_acquire_for(unsigned int _dur=1){
        std::scoped_lock lock(_mutex);
        return _semaphore.try_acquire_for(_dur * __timestep);
    }
    virtual bool try_pop(Data& p){
        std::scoped_lock lock(_mutex);
        auto res = _semaphore.try_acquire();
        if (res) {
            _data.pop(p);
        }
        return res;
    }
    virtual bool try_pop_for(Data& p, unsigned int _dur){
        std::scoped_lock lock(_mutex);
        auto res = _semaphore.try_acquire_for(_dur * __timestep);
        if (res) {
            _data.pop(p);
        }
        return res;
    }
    //    template<class Rep, class Period>
    //    bool try_acquire_for( const std::chrono::duration<Rep, Period>& rel_time ){
    //        return _semaphore.try_acquire_for(rel_time);
    //    }
private:
    std::mutex _mutex;
    DataQueue _data;
    Semaphore _semaphore;
};
} // namespace tbk;
#endif // __TBK_SEMADATA_H__
