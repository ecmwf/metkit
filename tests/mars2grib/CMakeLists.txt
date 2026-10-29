add_subdirectory( api )
add_subdirectory( utils )
add_subdirectory( backend )
add_subdirectory( encoding )

# Corpus ctests from a block file of test cases (-DENABLE_MARS2GRIB_ROUNDTRIP_TESTCASES=ON, off by default)
if( HAVE_MARS2GRIB_ROUNDTRIP_TESTCASES )
    add_subdirectory( testcases )
endif()
