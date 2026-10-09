# (C) Copyright 2026- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation
# nor does it submit to any jurisdiction.

# pymetkit on its own, against an installed metkit:
#   cmake -S python/pymetkit -B build -DCMAKE_PREFIX_PATH=<metkit and its deps>

cmake_minimum_required( VERSION 3.18 FATAL_ERROR )

project( pymetkit LANGUAGES CXX C )

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package( metkit REQUIRED )
find_package( Python 3.11 REQUIRED COMPONENTS Interpreter Development )
find_package( pybind11 3.0.1 REQUIRED )

set( metkit_VERSION_STR ${metkit_VERSION} )
set( HAVE_PYTHON_METKIT_INTERFACE ON )
set( HAVE_MARS2GRIB ${metkit_HAVE_MARS2GRIB} )
set( HAVE_MARS2MARS ${metkit_HAVE_MARS2MARS} )

include( ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/pymetkit.cmake )

enable_testing()
set( metkit_env "METKIT_HOME=$<TARGET_FILE_DIR:metkit>/.." )
add_subdirectory( ${CMAKE_CURRENT_SOURCE_DIR}/../../tests/pymetkit ${CMAKE_CURRENT_BINARY_DIR}/tests )
