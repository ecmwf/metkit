
ecbuild_add_test(
    TARGET
        mars2grib-localConfigurationTraits-tests

    SOURCES
        mars2grib-localConfigurationTraits-tests.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

ecbuild_add_test(

    TARGET
        mars2grib-codesHandleTraits-tests

    SOURCES
        mars2grib-codesHandleTraits-tests.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
 )