configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/mars2mars-test-requests.mars
    ${CMAKE_CURRENT_BINARY_DIR}/mars2mars-test-requests.mars
    COPYONLY
)

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

ecbuild_add_test(
    TARGET
        mars2mars-test-02

    SOURCES
        mars2mars-test-02.cc

    NO_AS_NEEDED

    LIBS
        eckit
        metkit
)

if( HAVE_BUILD_GRIB2_TOOLS )
    ecbuild_configure_file( convert-mars-request.sh.in convert-mars-request.sh @ONLY )

    ecbuild_add_test(
        TARGET   convert-mars-request
        TYPE     SCRIPT
        COMMAND  convert-mars-request.sh )
endif()
