#ifndef __TBK_DATANODE_H__
#define __TBK_DATANODE_H__
#include <cstdlib>

#ifdef TBK_DEBUG
#include "tbk/log.h"
#endif

namespace tbk{
class DataNode{
public:
    DataNode(DataNode* _last=nullptr,DataNode* _next=nullptr):_last(_last),_next(_next),_data(nullptr),_capacity(0),_size(0){
        #ifdef TBK_DEBUG
        tbk::log("{} TBK DataNode constructor\n",fmt::ptr(this));
        #endif
    }
    DataNode(const DataNode&)=delete;
    DataNode(DataNode&&)=default;
    virtual ~DataNode(){
        #ifdef TBK_DEBUG
        tbk::log("{} TBK DataNode destructor\n",fmt::ptr(this));
        #endif
        if(_capacity > 0){
            free(_data);
        }
    }
    virtual void resize(const unsigned long size){
        if(size > _capacity){
            if(_data != nullptr){
                free(_data);
            }
            _data = malloc(size);
            _capacity = size;
        }
        _size = size;
    }
    DataNode* _last;
    DataNode* _next;
    void* _data;
    unsigned long _capacity;
    unsigned long _size;
};
}
#endif // __TBK_DATANODE_H__
