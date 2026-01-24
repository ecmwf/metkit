
# ---------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------
list(APPEND mars2grib_capi_srcs
    mars2grib_c.cc
)

# ---------------------------------------------------------------------
# Object library
# ---------------------------------------------------------------------
if (HAVE_FORTRAN)

    ecbuild_add_library(

        TARGET mars2grib-capi-obj

        TYPE OBJECT

        SOURCES
            ${mars2grib_capi_srcs}

    )

    install(
        DESTINATION include/metkit/mars2grib/api/c
        FILES_MATCHING
            PATTERN "mars2grib_c.h"
    )

endif ()
