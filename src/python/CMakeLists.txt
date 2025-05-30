cmake_minimum_required(VERSION 3.14)
project(tbkpy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
find_package(tbk_core REQUIRED)
set(LIB_NAME _core)
file(GLOB SRC 
    ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}lib/${LIB_NAME}/*.cpp
)
find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(${LIB_NAME} ${SRC})
target_link_libraries(${LIB_NAME} PRIVATE pybind11::module tbk::tbk_core)