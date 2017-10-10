cmake_minimum_required( VERSION 2.8.11 FATAL_ERROR )

project( metkit CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 2.0 )

### dependencies

# grib support

ecbuild_add_option( FEATURE GRIB
                    DEFAULT ON
                    DESCRIPTION "Add support for GRIB format" )

ecbuild_add_option( FEATURE ECCODES
                    CONDITION HAVE_GRIB
                    DEFAULT ON
                    DESCRIPTION "Use ecCodes instead of grib_api"
                    REQUIRED_PACKAGES "PROJECT eccodes VERSION 2.5" )

if( HAVE_GRIB )

  if( HAVE_ECCODES )
    set( GRIB_API_INCLUDE_DIRS ${ECCODES_INCLUDE_DIRS} )
    set( GRIB_API_LIBRARIES    ${ECCODES_LIBRARIES} )
    set( GRIB_API_DEFINITIONS  ${ECCODES_DEFINITIONS} )
    set( grib_api_BASE_DIR     ${eccodes_BASE_DIR} )
    set( grib_handling_pkg     eccodes )
  else()
    ecbuild_use_package( PROJECT grib_api VERSION 1.24 )
    set( grib_handling_pkg     grib_api )
    if (GRIB_API_FOUND)
      ecbuild_warn("Note: Although grib_api can be used, we strongly advise you to upgrade to ecCodes. "
                   "See https://software.ecmwf.int/wiki/display/ECC/ecCodes+Home\n" )
    else()
      set(HAVE_GRIB 0)
      set( grib_handling_pkg ) # METK
      ecbuild_disable_feature( "GRIB" )
      if (ENABLE_GRIB MATCHES "REQUIRE" OR GRIB_user_provided_input)
        ecbuild_critical("Either eccodes or grib_api are required, but not found")
      endif()
    endif()
  endif()

endif()

# netcdf support

ecbuild_add_option( FEATURE NETCDF
  DESCRIPTION "Support for NetCDF data"
  DEFAULT ON
  REQUIRED_PACKAGES NetCDF
  NO_TPL
)

# eckit

ecbuild_use_package( PROJECT eckit  VERSION  0.18.0 REQUIRED )

### start project

ecbuild_declare_project()

### export package info

set( METKIT_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( METKIT_LIBRARIES      metkit )

get_directory_property( METKIT_DEFINITIONS COMPILE_DEFINITIONS )

### sources

include_directories( ${METKIT_INCLUDE_DIRS} )

add_subdirectory( src )
add_subdirectory( share )

### finalize project

ecbuild_pkgconfig( NAME metkit
                   DESCRIPTION "ECMWF Meteorological toolkit"
                   LIBRARIES metkit )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
