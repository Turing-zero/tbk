file(GLOB TEST_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test_*.cpp
    )

set(IGNORE_TEST
    )

if(PROCESS_COMM)
else()
    list(APPEND IGNORE_TEST
        test_etcd
        test_params
        )
endif()
message("--------build test--------")
foreach(test_file ${TEST_FILES})
    file(RELATIVE_PATH test_file_name ${CMAKE_CURRENT_SOURCE_DIR} ${test_file})
    string( REPLACE ".cpp" "" test_name ${test_file_name} )
    if(${test_name} IN_LIST IGNORE_TEST)
        message(${test_name} " ignore, no compile")
        continue()
    endif()
    message("build test : " ${test_file} " -- " ${test_name})

    add_executable(${test_name}
        ${test_file_name}
        )
    target_link_libraries(${test_name} tbk_core)
endforeach()