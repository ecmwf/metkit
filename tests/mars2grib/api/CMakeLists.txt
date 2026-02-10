
ecbuild_add_test( TARGET        mars2grib_test_cpp_api
                  CONDITION     HAVE_MARS2GRIB
                  INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                  SOURCES       test_cpp_api.cc
                  LIBS          metkit
                  NO_AS_NEEDED
)

ecbuild_add_test( TARGET        mars2grib_test_python_api
                  CONDITION     HAVE_MARS2GRIB_PYTHON
                  COMMAND       python3
                  ARGS          ${CMAKE_CURRENT_SOURCE_DIR}/test_python_api.py
                  ENVIRONMENT   PYTHONPATH=${CMAKE_BINARY_DIR}/lib
)
