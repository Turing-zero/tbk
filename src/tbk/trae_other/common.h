#pragma once
#include <string>
// 统一的Result类型
template<typename T>
class Result {
    bool success;
    T value;
    std::string error_message;
};