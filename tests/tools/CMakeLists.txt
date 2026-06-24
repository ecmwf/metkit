
if ( HAVE_GRIB AND HAVE_BUILD_TOOLS )

    list( APPEND metkit_tools
        grib-to-request
    )

    foreach( _test ${metkit_tools} )
        ecbuild_configure_file( ${_test}.sh.in ${_test}.sh @ONLY )

        ecbuild_add_test(
            TYPE     SCRIPT
            COMMAND  ${_test}.sh )

    endforeach( )

endif()
