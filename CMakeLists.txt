# (C) Copyright 2011- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

########################################################################################################################

cmake_minimum_required( VERSION 3.18 FATAL_ERROR )

find_package( ecbuild 3.12 REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild)

project( metkit LANGUAGES CXX C )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

########################################################################################################################
### dependencies and options

if(NOT METKIT_CONFIGS_BRANCH)
    set(METKIT_CONFIGS_BRANCH chk)
endif()

# eckit

set( PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "" ) # needed for generating .b files for persistent support

ecbuild_find_package( NAME eckit  VERSION  1.32 REQUIRED )


ecbuild_add_option( FEATURE BUILD_TOOLS
                    DEFAULT ON
                    DESCRIPTION "Build the command line tools" )

ecbuild_add_option( FEATURE BUILD_GRIB2_TOOLS
                    DEFAULT OFF
                    DESCRIPTION "Build the command line tools to support the migration to Grib2" )

ecbuild_add_option( FEATURE EXPERIMENTAL
                    DEFAULT OFF
                    DESCRIPTION "Experimental features" )
# GRIB support

ecbuild_add_option( FEATURE GRIB
                    DEFAULT ON
                    DESCRIPTION "Add support for GRIB format"
                    REQUIRED_PACKAGES "NAME eccodes VERSION 2.46" )

# BUFR support

ecbuild_add_option( FEATURE BUFR
                    DEFAULT ON
                    DESCRIPTION "Add support for BUFR format"
                    REQUIRED_PACKAGES "NAME eccodes VERSION 2.46" )

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

# MARS2GRIB encoder

ecbuild_add_option( FEATURE GRIB2MARS
                    DEFAULT ON
                    CONDITION HAVE_GRIB
                    DESCRIPTION "Build the GRIB2MARS converter" )

ecbuild_add_option( FEATURE MARS2MARS
                    DEFAULT ON
                    DESCRIPTION "Build the convert-to-grib2 MarsRequest converter" )

ecbuild_add_option( FEATURE MARS2GRIB
                    DEFAULT ON
                    CONDITION HAVE_GRIB AND eckit_HAVE_ECKIT_GEO
                    DESCRIPTION "Build the MARS2GRIB encoder" )

# Python bindings for mars2grib encoder
ecbuild_add_option( FEATURE MARS2GRIB_PYTHON
                    DEFAULT OFF
                    CONDITION HAVE_MARS2GRIB
                    DESCRIPTION "Build mars2grib python library"
                    REQUIRED_PACKAGES
                        "NAME Python VERSION 3.11 COMPONENTS Interpreter Development"
                        "NAME pybind11 VERSION 3.0.1" )

# Python bindings for mars2mars
ecbuild_add_option( FEATURE MARS2MARS_PYTHON
                    DEFAULT OFF
                    CONDITION HAVE_MARS2MARS
                    DESCRIPTION "Build mars2mars python library"
                    REQUIRED_PACKAGES
                        "NAME Python VERSION 3.11 COMPONENTS Interpreter Development"
                        "NAME pybind11 VERSION 3.0.1" )

# Pythonic pybind11 interface (pymetkit)

ecbuild_add_option( FEATURE PYTHON_METKIT_INTERFACE
                    DEFAULT OFF
                    DESCRIPTION "Build the pybind11 pymetkit interface"
                    REQUIRED_PACKAGES
                        "NAME Python VERSION 3.11 COMPONENTS Interpreter Development"
                        "NAME pybind11 VERSION 3.0.1" )

# We need PyMetkit in case MARS2GRIB python is enabled
if((HAVE_MARS2GRIB_PYTHON OR HAVE_MARS2MARS_PYTHON) AND NOT HAVE_PYTHON_METKIT_INTERFACE)
    message( WARN "MARS2GRIB or MARS2MARS python interface requires metkit python interface. I enable ENABLE_PYTHON_METKIT_INTERFACE to build the PyMetkit interface" )
    set(ENABLE_PYTHON_METKIT_INTERFACE "ON")
endif()


# METKIT config files support

ecbuild_add_option( FEATURE METKIT_CONFIG
                    DEFAULT ON
                    DESCRIPTION "Install metkit configuration files" )

# Temporary
ecbuild_add_option( FEATURE FAIL_ON_CCSDS
                    DESCRIPTION "Fail on CCSDS"
                    DEFAULT OFF	 )

### Documentation
ecbuild_add_option( FEATURE METKIT_DOCUMENTATION
                    DEFAULT OFF
                    DESCRIPTION "Enable documentation generation" )


############################################################################################
# sources

include(cmake/compiler_warnings.cmake) # optionally handle compiler specific warnings

add_subdirectory( src )
add_subdirectory( share )
add_subdirectory( tests )

if(ENABLE_METKIT_DOCUMENTATION)
    add_subdirectory(docs)
endif()

############################################################################################
# finalize

ecbuild_pkgconfig( NAME metkit
                   DESCRIPTION "ECMWF Meteorological toolkit"
                   LIBRARIES metkit )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
