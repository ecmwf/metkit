if( HAVE_MARS2MARS )

ecbuild_add_test(
    TARGET
        mars2mars-test-01

    SOURCES
        mars2mars-test-01.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

endif()
