### provides MARS requests from GRIB files

ecbuild_add_executable(

    TARGET   grib2request

    SOURCES  grib2request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${OOGRIB_INCLUDE_DIRS}

    LIBS     marskit
)
