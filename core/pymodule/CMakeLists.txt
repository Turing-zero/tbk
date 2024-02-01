set(SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp)

find_package(pybind11 CONFIG REQUIRED)

add_library(tbkpy MODULE ${SOURCES})
set_target_properties(tbkpy PROPERTIES 
    PREFIX "${PYTHON_MODULE_PREFIX}"
    # SUFFIX "${PYTHON_MODULE_EXTENSION}"
    )
file(GLOB_RECURSE PY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.py)
add_custom_target(copy_py ALL DEPENDS ${PY_FILES})
add_custom_command(TARGET copy_py
    COMMAND ${CMAKE_COMMAND} -E copy
    ${PY_FILES}
    ${CMAKE_BINARY_DIR}
    )
target_link_libraries(tbkpy PRIVATE pybind11::module tbk_core)
