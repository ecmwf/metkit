# (C) Copyright 2011- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

########################################################################################################################

cmake_minimum_required( VERSION 3.12 FATAL_ERROR )

find_package( ecbuild 3.7.2 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild)

project( metkit LANGUAGES CXX )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

########################################################################################################################
### dependencies and options

ecbuild_add_option( FEATURE BUILD_TOOLS
                    DEFAULT ON
                    DESCRIPTION "Build the command line tools" )

ecbuild_add_option( FEATURE EXPERIMENTAL
                    DEFAULT OFF
                    DESCRIPTION "Experimental features" )
# GRIB support

ecbuild_add_option( FEATURE GRIB
                    DEFAULT ON
                    DESCRIPTION "Add support for GRIB format"
                    REQUIRED_PACKAGES "NAME eccodes VERSION 2.27" )

# BUFR support

ecbuild_add_option( FEATURE BUFR
                    DEFAULT ON
                    DESCRIPTION "Add support for BUFR format"
                    REQUIRED_PACKAGES "NAME eccodes VERSION 2.27" )

# netcdf support

ecbuild_add_option( FEATURE NETCDF
  DEFAULT ON
  DESCRIPTION "Support for NetCDF data"
  REQUIRED_PACKAGES NetCDF
)

# ODB support

ecbuild_add_option( FEATURE ODB
                    DEFAULT ON
                    DESCRIPTION "Add support for ODB data"
                    REQUIRED_PACKAGES "NAME odc VERSION 1.0" )

# Temporary
ecbuild_add_option( FEATURE FAIL_ON_CCSDS
                    DESCRIPTION "Fail on CCSDS"
                    DEFAULT OFF	 )
# eckit

set( PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "" ) # needed for generating .b files for persistent support

ecbuild_find_package( NAME eckit  VERSION  1.16 REQUIRED )

############################################################################################
# sources

include(cmake/compiler_warnings.cmake) # optionally handle compiler specific warnings

add_subdirectory( src )
add_subdirectory( share )
add_subdirectory( tests )

############################################################################################
# finalize

ecbuild_pkgconfig( NAME metkit
                   DESCRIPTION "ECMWF Meteorological toolkit"
                   LIBRARIES metkit )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
