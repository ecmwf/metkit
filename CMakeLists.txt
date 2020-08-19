cmake_minimum_required( VERSION 3.12 FATAL_ERROR )

project( metkit VERSION 1.5.10 LANGUAGES CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 3.4 )

ecbuild_declare_project()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

### dependencies and options

ecbuild_add_option( FEATURE EXPERIMENTAL
                    DEFAULT OFF
                    DESCRIPTION "Experimental features" )
# GRIB support

ecbuild_add_option( FEATURE GRIB
                    DEFAULT ON
                    DESCRIPTION "Add support for GRIB format (grib_api not supported)"
                    REQUIRED_PACKAGES "NAME eccodes VERSION 2.5" )

# BUFR support

ecbuild_add_option( FEATURE BUFR
                    DEFAULT OFF
                    DESCRIPTION "Add support for BUFR4 format"
                    REQUIRED_PACKAGES "NAME eccodes VERSION 2.8" )

# netcdf support

ecbuild_add_option( FEATURE NETCDF
  DEFAULT ON
  DESCRIPTION "Support for NetCDF data"
  REQUIRED_PACKAGES NetCDF
  NO_TPL
)

# ODB support

ecbuild_add_option( FEATURE ODB
                    DEFAULT ON
                    DESCRIPTION "Add support for ODB data"
                    REQUIRED_PACKAGES "NAME odc VERSION 1.0" )

# eckit

set( PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "" ) # needed for generating .b files for persistent support

ecbuild_find_package( NAME eckit  VERSION  1.11 REQUIRED )

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
