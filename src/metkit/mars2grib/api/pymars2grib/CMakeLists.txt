pybind11_add_module(mars2grib_core MODULE mars2grib_core.cc)

target_link_libraries(mars2grib_core
    PRIVATE pybind11::module
            pybind11::lto
            metkit
            eckit
)

if(NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
    # Strip unnecessary sections of the binary on Linux/macOS
    pybind11_strip(mars2grib_core)
endif()

set_target_properties(mars2grib_core
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

install(
  TARGETS mars2grib_core
  COMPONENT python
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)

# Copy mars2grib.py into the install directory for now
# TODO: Create a python wheel which can be installed with pip
install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/mars2grib.py
  COMPONENT python
  DESTINATION "${CMAKE_INSTALL_LIBDIR}"
)
