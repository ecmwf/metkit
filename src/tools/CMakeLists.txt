### provides MARS requests from GRIB files

ecbuild_add_executable(

    TARGET   client_cli

    SOURCES  client_cli.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${ODB_API_INCLUDE_DIRS}

    LIBS     marskit ${odblibs} 
)

ecbuild_add_executable(

    TARGET   grib2request

    CONDITION HAVE_GRIBPP

    SOURCES  grib2request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${GRIBPP_INCLUDE_DIRS}

    LIBS     marskit
)
