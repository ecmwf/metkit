# METKIT config files support
ecbuild_add_option( FEATURE METKIT_CONFIG
                    DEFAULT ON
                    DESCRIPTION "Install metkit configuration files" )

add_subdirectory( metkit )

ecbuild_install_project( NAME metkit-config )