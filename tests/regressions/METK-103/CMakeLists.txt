ecbuild_configure_file( METK-103.sh.in METK-103.sh @ONLY )

ecbuild_add_test(
    TYPE      SCRIPT
    CONDITION HAVE_BUFR AND HAVE_BUILD_TOOLS
    COMMAND   METK-103.sh )

