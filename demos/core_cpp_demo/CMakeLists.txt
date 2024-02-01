cmake_minimum_required(VERSION 3.14)

project(TestTBK LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(tbk_core REQUIRED)
message("markdebug : " ${tbk_core_FOUND})
message("markdebug : " ${tbk_core_INCLUDE_DIRS})
message("markdebug : " ${tbk_core_LIBRARIES})
add_executable(TestTBK main.cpp)
target_link_libraries(TestTBK PRIVATE tbk::tbk_core)