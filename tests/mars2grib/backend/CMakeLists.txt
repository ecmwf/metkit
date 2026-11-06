
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
        mars2grib-test-recipes-probability-quantile

    SOURCES
        mars2grib-test-recipes-probability-quantile.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)
