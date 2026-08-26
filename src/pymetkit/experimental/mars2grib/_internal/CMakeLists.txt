pybind11_add_module(mars2grib_bindings MODULE mars2grib_core.cc)

target_link_libraries(mars2grib_bindings
    PRIVATE pybind11::module
            pybind11::lto
            metkit
            eckit
)

if(NOT MSVC AND NOT "${CMAKE_BUILD_TYPE}" MATCHES "Debug|RelWithDebInfo")
    # Strip unnecessary sections of the binary on Linux/macOS
    pybind11_strip(mars2grib_bindings)
endif()

set(INTERNAL_PATH ${PYMETKIT_STAGING_PYMETKIT_EXPERIMENTAL}/mars2grib/_internal)

set_target_properties(mars2grib_bindings
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${INTERNAL_PATH}
)


