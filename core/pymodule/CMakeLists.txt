set(SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/module.cpp)

find_package(pybind11 CONFIG REQUIRED)
pybind11_add_module(tbkpylib ${SOURCES})
set_target_properties(tbkpylib PROPERTIES
    PREFIX "${PYTHON_MODULE_PREFIX}"
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tbkpy
    )
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/tbkpy DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pyproject.toml ${CMAKE_CURRENT_BINARY_DIR}/pyproject.toml)
target_link_libraries(tbkpylib PRIVATE pybind11::module tbk_core)
