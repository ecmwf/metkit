
# ---------------------------------------------------------------------
# Sources
# ---------------------------------------------------------------------
list(APPEND mars2grib_fapi_srcs
    mars2grib_constants_mod.F90
    mars2grib_utils_mod.F90
    mars2grib_dictionaries_mod.F90
    mars2grib_encoder_mod.F90
    mars2grib_mod.F90
)

# ---------------------------------------------------------------------
# Object library
# ---------------------------------------------------------------------
if (HAVE_FORTRAN)

    ecbuild_add_library(

        TARGET mars2grib-fapi-obj

        TYPE OBJECT

        SOURCES
            ${mars2grib_fapi_srcs}

        PUBLIC_INCLUDES
            $<BUILD_INTERFACE:${CMAKE_Fortran_MODULE_DIRECTORY}>
            $<INSTALL_INTERFACE:include/metkit/mars2grib/api/fortran>

    )

    install(
        DIRECTORY ${CMAKE_Fortran_MODULE_DIRECTORY}/
        DESTINATION include/metkit/mars2grib/api/fortran
        FILES_MATCHING
            PATTERN "*.mod"
    )

endif ()
