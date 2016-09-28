cmake_minimum_required( VERSION 2.8.12 FATAL_ERROR )

project( metkit CXX )

set( CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/../ecbuild/cmake")

include( ecbuild_system NO_POLICY_SCOPE )

ecbuild_requires_macro_version( 2.0 )

### dependencies

set( PERSISTENT_NAMESPACE "eckit" CACHE INTERNAL "" )

ecbuild_use_package( PROJECT eckit  VERSION  0.14 REQUIRED )

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
                   DESCRIPTION "ECMWF MARS toolkit"
                   LIBRARIES metkit )

ecbuild_install_project( NAME ${PROJECT_NAME} )

ecbuild_print_summary()
