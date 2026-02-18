
ecbuild_add_test(
    TARGET
        mars2grib-test-search

    SOURCES
        mars2grib-test-search.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)
