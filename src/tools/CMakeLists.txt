### provides MARS requests from GRIB files

ecbuild_add_executable(

    TARGET   grib-to-mars-request

    CONDITION HAVE_GRIB

    SOURCES  grib-to-mars-request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}

    LIBS     metkit
)

ecbuild_add_executable(

    TARGET   parse-mars-request

    # CONDITION HAVE_GRIB

    SOURCES  parse-mars-request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}

    LIBS     metkit
)
