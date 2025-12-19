ecbuild_add_executable(
    TARGET   test-mars2grib-api
    SOURCES  test_mars2grib_api.cc
    INCLUDES ${ECKIT_INCLUDE_DIRS}
    LIBS     metkit eckit_option eckit
    NO_AS_NEEDED
)
