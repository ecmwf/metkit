pybind11_add_module(mars2mars_bindings MODULE mars2mars_bindings.cc)

target_link_libraries(mars2mars_bindings
    PRIVATE pybind11::module
            pybind11::lto
            metkit
            eckit
)

if(NOT MSVC AND NOT "${CMAKE_BUILD_TYPE}" MATCHES "Debug|RelWithDebInfo")
    # Strip unnecessary sections of the binary on Linux/macOS
    pybind11_strip(mars2mars_bindings)
endif()

set(INTERNAL_PATH ${PYMETKIT_STAGING_PYMETKIT_EXPERIMENTAL}/mars2mars/_internal)

set_target_properties(mars2mars_bindings
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${INTERNAL_PATH}
)


