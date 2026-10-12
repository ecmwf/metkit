foreach( _src
        001_oper_fc_as_control.cc
        002_spherical_harmonics.cc
        003_regular_latlon.cc
        004_regular_gaussian.cc
        005_reduced_gaussian.cc
        006_healpix.cc
        007_bitsPerValue.cc
        008_pressure_level.cc
        009_enfh_chemical_reforecast.cc
        010_oper_enfo_chemical.cc
        011_enfh_wave_reforecast.cc
        012_seas6_stat_failure.cc
        013_aifs-single_as_non_ensemble.cc
        014_seas6_ensemble.cc
    )
    get_filename_component( _name "${_src}" NAME_WE )

    ecbuild_add_test( TARGET        mars2grib_test_encoding_${_name}_legacy
                      SOURCES       ${_src}
                      LIBS          metkit
                      NO_AS_NEEDED
    )

    ecbuild_add_test( TARGET        mars2grib_test_encoding_${_name}_gridSpec
                      SOURCES       ${_src}
                      LIBS          metkit
                      ENVIRONMENT   ECCODES_ECKIT_GEO=1
                      NO_AS_NEEDED
    )
endforeach()
