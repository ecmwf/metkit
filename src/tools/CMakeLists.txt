### provides MARS requests from GRIB files

ecbuild_add_executable(

    TARGET   client_cli

    SOURCES  client_cli.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}

    LIBS     marskit
)


ecbuild_add_executable(

    TARGET   grib2request

    SOURCES  grib2request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${GRIBPP_INCLUDE_DIRS}

    LIBS     marskit
)
