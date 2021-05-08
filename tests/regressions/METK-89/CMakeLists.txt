ecbuild_configure_file( METK-89.sh.in METK-89.sh @ONLY )

ecbuild_add_test(
    TYPE     SCRIPT
    COMMAND  METK-89.sh )

