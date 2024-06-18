cmake_minimum_required(VERSION 3.14)

project(core LANGUAGES CXX VERSION 0.2.0)

string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%d %H:%M:%S")
message("build timestamp: " ${BUILD_TIMESTAMP})

option(TBK_DEBUG "tbk debug output." OFF)
option(TBK_USE_CUSTOM_SEMAPHORE "use custom semaphore." ON)
option(PROCESS_COMM "PROCESS - on:process level comm,off:only thread level comm ." ON)
option(BUILD_PYTHON_MODULE "build python module." OFF)
option(BUILD_TEST "build test." OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set(CMAKE_AUTOUIC ON)
# set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

if(TBK_DEBUG)
    message("set TBK_DEBUG -- ON")
    add_definitions(-DTBK_DEBUG)
    # add_definitions(-DTBK_THREADPOOL_DEBUG)
else()
    message("set TBK_DEBUG -- OFF")
endif()
if(BUILD_PYTHON_MODULE)
    message("set BUILD_PYTHON_MODULE -- ON")
    add_definitions(-DBUILD_PYTHON_MODULE)
else()
    message("set BUILD_PYTHON_MODULE -- OFF")
endif()

# generate version header
execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --match "v*"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
    RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    RESULT_VARIABLE GIT_HASH_ERROR_CODE
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
if(NOT GIT_DESCRIBE_ERROR_CODE AND NOT GIT_HASH_ERROR_CODE)
    set(TBK_VERSION "${GIT_DESCRIBE_VERSION}.dev+${GIT_HASH}")
elseif(NOT GIT_HASH_ERROR_CODE)
    set(TBK_VERSION "0.0.dev+${GIT_HASH}")
else()
    set(TBK_VERSION "0.0.dev")
endif()
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tbk/version.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tbk/version.h
    @ONLY
    )

# find package
find_package(fmt REQUIRED)
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)
find_package(yaml-cpp REQUIRED)

# pb & grpc file
set(TZCP_DIR ${CMAKE_SOURCE_DIR}/../tzcp)
set(PB_OUTPUT_DIR ${CMAKE_BINARY_DIR}/tbk/protocol)
file(MAKE_DIRECTORY ${PB_OUTPUT_DIR})
file(GLOB PB_FILES 
    ${TZCP_DIR}/tbk/*.proto
)
message("pb input dir: " ${TZCP_DIR})
message("pb files: " ${PB_FILES})
if(PROCESS_COMM)
    message("set PROCESS_COMM -- ON")
    add_definitions(-DTBK_PROCESS_COMM)
    find_package(Boost COMPONENTS system thread random REQUIRED)
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GRPC REQUIRED grpc)
    pkg_search_module(GRPCPP REQUIRED grpc++)
    # find_package(gRPC REQUIRED)
    find_package(etcd-cpp-api)
    message("etcd-cpp-api found: " ${etcd-cpp-api_FOUND})
    ## use ExternalProject_Add if not found gRPC or etcd-cpp-api
    if(NOT etcd-cpp-api_FOUND)
        message("etcd-cpp-api not found, use ExternalProject_Add")
        include(ExternalProject)
        ExternalProject_Add(etcd-cpp-api
            GIT_REPOSITORY https://github.com/etcd-cpp-apiv3/etcd-cpp-apiv3
            GIT_TAG v0.15.4
            SOURCE_DIR ${CMAKE_BINARY_DIR}/3rdparty/source/etcd-cpp-api
            CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/3rdparty/
        )
        set(ETCD_CPP_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/3rdparty/include)
        set(ETCD_CPP_LIBRARIES ${CMAKE_BINARY_DIR}/3rdparty/lib/libetcd-cpp-api.so)
    endif()
    # message("etcd-cpp-api include dir: " ${ETCD_CPP_INCLUDE_DIRS})
    # message("etcd-cpp-api lib: " ${ETCD_CPP_LIBRARIES})
    find_program(_GRPC_CPP_PLUGIN_EXECUTABLE grpc_cpp_plugin)
    foreach(file ${PB_FILES})
        execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} 
        --proto_path=${TZCP_DIR}/tbk
        --cpp_out=${PB_OUTPUT_DIR}
        --grpc_out=${PB_OUTPUT_DIR}
        --plugin=protoc-gen-grpc=${_GRPC_CPP_PLUGIN_EXECUTABLE}
        ${file})
    endforeach()
    function(get_grpc_file H CPP)
        list(APPEND PH ${ARGN})
        list(APPEND PC ${ARGN})
        list(TRANSFORM PH PREPEND ${PB_OUTPUT_DIR}/)
        list(TRANSFORM PH APPEND .grpc.pb.h)
        list(TRANSFORM PC PREPEND ${PB_OUTPUT_DIR}/)
        list(TRANSFORM PC APPEND .grpc.pb.cc)
        set(${H} ${PH} PARENT_SCOPE)
        set(${CPP} ${PC} PARENT_SCOPE)
    endfunction()
else()
    message("set PROCESS_COMM -- OFF")
    foreach(file ${PB_FILES})
        execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} 
        --proto_path=${PB_INPUT_DIR}
        --cpp_out=${PB_OUTPUT_DIR}
        ${file})
    endforeach()
endif()
function(get_pb_file H CPP)
	list(APPEND PH ${ARGN})
	list(APPEND PC ${ARGN})
	list(TRANSFORM PH PREPEND ${PB_OUTPUT_DIR}/)
	list(TRANSFORM PH APPEND .pb.h)
	list(TRANSFORM PC PREPEND ${PB_OUTPUT_DIR}/)
	list(TRANSFORM PC APPEND .pb.cc)
	set(${H} ${PH} PARENT_SCOPE)
	set(${CPP} ${PC} PARENT_SCOPE)
endfunction()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}
    ${fmt_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${YAML_CPP_INCLUDE_DIRS}
    ${ETCD_CPP_INCLUDE_DIRS}
)
if(TBK_USE_CUSTOM_SEMAPHORE)
    add_definitions(-DTBK_USE_CUSTOM_SEMAPHORE)
endif()

file(GLOB TBK_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tbk/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/include/tbk/utils/*.h
    )
# check platform
if(UNIX)
    if(APPLE)
        message("platform is apple")
        set(POSIX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/posix/posix_apple.cpp)
    else()
        message("platform is linux")
        set(POSIX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/posix/posix_linux.cpp)
    endif()
elseif(WIN32)
    message("platform is windows")
    set(POSIX_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/posix/posix_windows.cpp)
else()
    message("platform is unknown")
endif()

file(GLOB TBK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
if(PROCESS_COMM)
    file(GLOB TBK_PROCESS_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/process/*.cpp)
    list(APPEND TBK_SOURCES ${TBK_PROCESS_SOURCES})
endif()
set(CPP_FILES
    ${POSIX_FILES}
    ${TBK_SOURCES}
    )

set(LIBS
    Threads::Threads
    fmt::fmt
    ${YAML_CPP_LIBRARIES}
    )

get_pb_file(PB_HEADER PB_CPP tbk tbk_test tbk_rpc)
if(PROCESS_COMM)
    get_grpc_file(RPC_HEADER RPC_CPP tbk_rpc tbk)
else()
    set(RPC_HEADER)
    set(RPC_CPP)
endif()

set(SOURCE_FILES
    ${CPP_FILES}
    ${PB_CPP}
    )
set(HEADER_FILES
    ${TBK_HEADERS}
    ${PB_HEADER}
    )
set(ALL_FILES
    ${SOURCE_FILES}
    ${HEADER_FILES}
)

set(addition_libs)

set(addition_libs
    ${Protobuf_LIBRARY}
    # gRPC::grpc++
    # gRPC::grpc++_reflection
    ${ETCD_CPP_LIBRARIES}
    )

add_library(tbk_core SHARED ${SOURCE_FILES})
if(PROCESS_COMM)
    add_dependencies(tbk_core etcd-cpp-api)
endif()
target_link_libraries(tbk_core INTERFACE ${LIBS} ${addition_libs})
target_include_directories(tbk_core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
    $<INSTALL_INTERFACE:include>)
# add_library(tbk_core_static STATIC ${SOURCE_FILES})
# target_link_libraries(tbk_core_static INTERFACE ${LIBS} ${addition_libs})
# target_include_directories(tbk_core INTERFACE
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
#     $<INSTALL_INTERFACE:include>)

if(BUILD_PYTHON_MODULE)
    add_subdirectory(pymodule)
endif()

install(TARGETS tbk_core
    EXPORT tbk_core-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/tbk DESTINATION include)
install(FILES ${PB_HEADER} ${RPC_HEADER} DESTINATION include/tbk/protocol)
install(EXPORT tbk_core-targets
    FILE tbk_core-targets.cmake
    NAMESPACE tbk::
    DESTINATION lib/cmake/tbk_core
    )
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/tbk_core-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tbk_core-config.cmake
    INSTALL_DESTINATION lib/cmake/tbk_core
    )
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/tbk_core-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
    )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tbk_core-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/tbk_core-config-version.cmake DESTINATION lib/cmake/tbk_core)

add_subdirectory(tool)
if(BUILD_TEST)
    add_subdirectory(test)
endif()
