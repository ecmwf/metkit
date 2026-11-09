
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

ecbuild_add_test(
    TARGET
        mars2grib-test-index-statistics

    SOURCES
        mars2grib-test-index-statistics.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)
