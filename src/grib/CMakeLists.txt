### grib sources

list( APPEND grib_srcs
    EmosFile.h
    EmosFile.cc
    GribFile.h
    GribFile.cc
    GribAccessor.h
    GribAccessor.cc
    GribMutator.h
    GribMutator.cc
    GribIndex.h
    GribIndex.cc
    GribHandle.h
    GribHandle.cc
    GribDataBlob.h
    GribDataBlob.cc
    GribMetaData.h
    GribMetaData.cc
)

### grib library
set( grib_handling_pkg grib_api )
if( HAVE_ECCODES )
  set( grib_handling_pkg eccodes )
endif()

ecbuild_add_library(

    TARGET grib

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/grib

    SOURCES
        ${grib_srcs}

    PRIVATE_INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${GRIB_API_INCLUDE_DIRS}

    LIBS
        eckit ${grib_handling_pkg}
)
