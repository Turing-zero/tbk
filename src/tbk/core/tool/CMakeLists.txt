file(GLOB TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/tool_*.cpp
    )
set(IGNORE_TEST
)
if(NOT PROCESS_COMM)
    list(APPEND IGNORE_TEST
        getInfo
        udpReceiver
        udpSender
    )
endif()
message("--------build tool--------")
foreach(TEST_FILE ${TEST_FILES})
    file(RELATIVE_PATH TEST_NAME ${CMAKE_CURRENT_SOURCE_DIR} ${TEST_FILE})
    string(REGEX REPLACE "tool_" "" TEST_NAME ${TEST_NAME})
    string(REPLACE ".cpp" "" TEST_NAME ${TEST_NAME})
    if(${TEST_NAME} IN_LIST IGNORE_TEST)
        message(${TEST_NAME} " ignore, no compile")
        continue()
    endif()
    message("build tool: ${TEST_NAME}")
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_link_libraries(${TEST_NAME} tbk_core)
endforeach()