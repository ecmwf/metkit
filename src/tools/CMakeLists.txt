### provides MARS requests from GRIB files

ecbuild_add_executable(

    TARGET   grib2request

    CONDITION HAVE_GRIBPP

    SOURCES  grib2request.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${GRIBPP_INCLUDE_DIRS}

    LIBS     marskit
)
