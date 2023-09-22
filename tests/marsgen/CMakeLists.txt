
list( APPEND marsgen_tests
    simple
    compare
    obs
    legacy)


foreach( _test ${marsgen_tests} )

    configure_file( metkit_marsgen_${_test}.sh.in metkit_marsgen_${_test}.sh @ONLY )

    ecbuild_add_test(
        TYPE     SCRIPT
        COMMAND  metkit_marsgen_${_test}.sh )

endforeach()