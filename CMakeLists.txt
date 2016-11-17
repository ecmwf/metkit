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
                    REQUIRED_PACKAGES "PROJECT eccodes VERSION 0.14" )

if( HAVE_GRIB AND HAVE_ECCODES )
  set( GRIB_API_INCLUDE_DIRS ${ECCODES_INCLUDE_DIRS} )
  set( GRIB_API_LIBRARIES    ${ECCODES_LIBRARIES} )
  set( GRIB_API_DEFINITIONS  ${ECCODES_DEFINITIONS} )
  set( grib_api_BASE_DIR     ${eccodes_BASE_DIR} )
  set( GRIB_API_FOUND        1 )
else()
  ecbuild_use_package( PROJECT grib_api VERSION 1.14 )
endif()

if(HAVE_GRIB AND NOT GRIB_API_FOUND)
    set(HAVE_GRIB 0)
endif()

# eckit

set( PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "" )

ecbuild_use_package( PROJECT eckit  VERSION  0.13 REQUIRED )

### start project

ecbuild_declare_project()

### export package info

set( METKIT_INCLUDE_DIRS   ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_BINARY_DIR}/src )
set( METKIT_LIBRARIES      metkit )

get_directory_property( METKIT_DEFINITIONS COMPILE_DEFINITIONS )

### sources

include_directories( ${METKIT_INCLUDE_DIRS} )

add_subdirectory( src )

### finalize project

ecbuild_pkgconfig( NAME metkit
                   DESCRIPTION "ECMWF Meteorological toolkit"
                   LIBRARIES metkit )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
