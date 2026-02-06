
ecbuild_add_test( TARGET        mars2grib_test_cpp_api
                  CONDITION     HAVE_MARS2GRIB
                  INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                  SOURCES       test_cpp_api.cc
                  LIBS          metkit
                  NO_AS_NEEDED
)


