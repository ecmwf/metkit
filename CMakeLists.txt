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

if(NOT METKIT_CONFIGS_BRANCH)
    set(METKIT_CONFIGS_BRANCH enum)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/share")
    if(IS_SYMLINK "${CMAKE_CURRENT_SOURCE_DIR}/share")
        if (METKIT_CONFIGS_PATH)
            #check if it matches with METKIT_CONFIGS_PATH
            file(REAL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/share" current_metkit_config_path EXPAND_TILDE)
            file(REAL_PATH "${METKIT_CONFIGS_PATH}" requested_metkit_config_path EXPAND_TILDE)
            if(NOT (current_metkit_config_path STREQUAL requested_metkit_config_path))
                message(SEND_ERROR "metkit-configs refers to ${current_metkit_config_path}; requested METKIT_CONFIGS_PATH is ${requested_metkit_config_path}")
            endif()
        endif()
    elseif(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/share")
        # update the content
        ecbuild_bundle( PROJECT share    GIT "https://github.com/ecmwf/metkit-configs"    BRANCH "${METKIT_CONFIGS_BRANCH}" UPDATE )
    else()
        message(SEND_ERROR "${CMAKE_CURRENT_SOURCE_DIR}/share must point to a valid metkit configuration. Cannot be a regular file")
    endif()
else()
    if(METKIT_CONFIGS_PATH)
        file( CREATE_LINK "${METKIT_CONFIGS_PATH}" "${CMAKE_CURRENT_SOURCE_DIR}/share" SYMBOLIC )
    else()
        ecbuild_bundle( PROJECT share    GIT "https://github.com/ecmwf/metkit-configs"    BRANCH "${METKIT_CONFIGS_BRANCH}" UPDATE )
    endif()
endif()

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
add_subdirectory( tests )

############################################################################################
# finalize

ecbuild_pkgconfig( NAME metkit
                   DESCRIPTION "ECMWF Meteorological toolkit"
                   LIBRARIES metkit )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
