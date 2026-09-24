list( APPEND MARS2GRIB_ENCODING_TESTS
    001_oper_fc_as_control
    002_spherical_harmonics
    003_regular_latlon
    004_regular_gaussian
    005_reduced_gaussian
    006_healpix
    007_bitsPerValue
    008_enfh_chemical_reforecast
)

list( SORT MARS2GRIB_ENCODING_TESTS )

foreach( _name IN LISTS MARS2GRIB_ENCODING_TESTS )
    ecbuild_add_test( TARGET        mars2grib_test_encoding_${_name}_legacy
                      CONDITION     HAVE_MARS2GRIB
                      INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                      SOURCES       "${_name}.cc"
                      LIBS          metkit
                      NO_AS_NEEDED
    )

    ecbuild_add_test( TARGET        mars2grib_test_encoding_${_name}_gridSpec
                      CONDITION     HAVE_MARS2GRIB
                      INCLUDES      "${ECCODES_INCLUDE_DIRS}"
                      SOURCES       "${_name}.cc"
                      LIBS          metkit
                      ENVIRONMENT   ECCODES_ECKIT_GEO=1
                      NO_AS_NEEDED
    )
endforeach()
