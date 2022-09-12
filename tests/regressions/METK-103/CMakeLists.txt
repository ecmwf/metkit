ecbuild_configure_file( METK-103.sh.in METK-103.sh @ONLY )

ecbuild_add_test(
    TYPE     SCRIPT
    COMMAND  METK-103.sh )

