### config headers

ecbuild_generate_config_headers( DESTINATION ${INSTALL_INCLUDE_DIR}/metkit )

configure_file( metkit_config.h.in    metkit_config.h )
configure_file( metkit_version.h.in   metkit_version.h )

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/metkit_config.h
            ${CMAKE_CURRENT_BINARY_DIR}/metkit_version.h
        DESTINATION
            ${INSTALL_INCLUDE_DIR}/metkit )

### metkit sources

list( APPEND metkit_srcs
    metkit_version.c
    config/LibMetkit.cc
    config/LibMetkit.h
    mars/BaseProtocol.cc
    mars/BaseProtocol.h
    mars/ClientTask.cc
    mars/ClientTask.h
    mars/DHSProtocol.cc
    mars/DHSProtocol.h
    mars/Matcher.cc
    mars/Matcher.h
    mars/MarsExpansion.cc
    mars/MarsExpansion.h
    mars/MarsExpension.h
    mars/MarsHandle.cc
    mars/MarsHandle.h
    mars/MarsLanguage.cc
    mars/MarsLanguage.h
    mars/MarsLocation.cc
    mars/MarsLocation.h
    mars/MarsParsedRequest.cc
    mars/MarsParsedRequest.h
    mars/MarsParser.cc
    mars/MarsParser.h
    mars/MarsRequest.cc
    mars/MarsRequest.h
    mars/MarsRequestHandle.cc
    mars/MarsRequestHandle.h
    mars/Parameter.cc
    mars/Parameter.h
    mars/ParamID.cc
    mars/ParamID.h
    mars/Quantile.cc
    mars/Quantile.h
    mars/RequestEnvironment.cc
    mars/RequestEnvironment.h
    mars/StepRangeNormalise.h
    mars/Type.cc
    mars/Type.h
    mars/TypeAny.cc
    mars/TypeAny.h
    mars/TypeDate.cc
    mars/TypeDate.h
    mars/TypeEnum.cc
    mars/TypeEnum.h
    mars/TypeExpver.cc
    mars/TypeExpver.h
    mars/TypeFloat.cc
    mars/TypeFloat.h
    mars/TypeInteger.cc
    mars/TypeInteger.h
    mars/TypeLowercase.cc
    mars/TypeLowercase.h
    mars/TypeMixed.cc
    mars/TypeMixed.h
    mars/TypeParam.cc
    mars/TypeParam.h
    mars/TypeRange.cc
    mars/TypeRange.h
    mars/TypeRegex.cc
    mars/TypeRegex.h
    mars/TypesFactory.cc
    mars/TypesFactory.h
    mars/TypeTime.cc
    mars/TypeTime.h
    mars/TypeToByList.h
    mars/TypeToByListQuantile.cc
    mars/TypeToByListQuantile.h
    tool/MetkitTool.cc
    tool/MetkitTool.h
    fields/FieldIndex.cc
    fields/FieldIndex.h
    fields/FieldIndexList.cc
    fields/FieldIndexList.h
    fields/SimpleFieldIndex.cc
    fields/SimpleFieldIndex.h
    hypercube/HyperCube.cc
    hypercube/HyperCube.h
    hypercube/HyperCubePayloaded.h
    api/metkit_c.cc
    api/metkit_c.h
)

list( APPEND metkit_persistent_srcs
    mars/Param.cc
    mars/Param.h
    mars/Param
    mars/StepRange.cc
    mars/StepRange.h
    mars/StepRange
)
list( APPEND metkit_srcs ${metkit_persistent_srcs} )

if ( HAVE_GRIB )

    list( APPEND metkit_srcs
        pointdb/bits.h
        pointdb/DataSource.cc
        pointdb/DataSource.h
        pointdb/FieldIndexer.cc
        pointdb/FieldIndexer.h
        pointdb/GribDataSource.cc
        pointdb/GribDataSource.h
        pointdb/GribFieldInfo.cc
        pointdb/GribFieldInfo.h
        pointdb/GribHandleDataSource.cc
        pointdb/GribHandleDataSource.h
        pointdb/masks.h
        pointdb/PointIndex.cc
        pointdb/PointIndex.h
        codes/CodesDecoder.h
        codes/BUFRDecoder.cc
        codes/BUFRDecoder.h
        codes/GRIBDecoder.cc
        codes/GRIBDecoder.h
        codes/CodesContent.cc
        codes/CodesContent.h
        codes/BufrContent.cc
        codes/BufrContent.h
        codes/UserDataContent.cc
        codes/UserDataContent.h
        codes/DataContent.cc
        codes/DataContent.h
        codes/MallocCodesContent.cc
        codes/MallocCodesContent.h
        codes/CodesSplitter.cc
        codes/CodesSplitter.h
        codes/GribAccessor.cc
        codes/GribAccessor.h
        codes/GribHandle.cc
        codes/GribHandle.h
        codes/GribIterator.cc
        codes/GribIterator.h
        codes/CodesHandleDeleter.h

        codes/api/CodesAPI.h
        codes/api/CodesAPI.cc
        codes/api/CodesTypes.h
        codes/api/CodesTypes.cc
        codes/api/KeyIterator.h
        codes/api/KeyIterator.cc
        codes/api/GeoIterator.h
    )

    if( NOT eccodes_HAVE_GEOGRAPHY OR NOT eccodes_HAVE_ECKIT_GEO )
        list( APPEND metkit_srcs
            codes/LibEccodes.cc
            codes/LibEccodes.h
        )
    endif()

    set( grib_libs eccodes )

endif ()

if ( HAVE_ODB )

    list( APPEND metkit_srcs
            odb/IdMapper.cc
            odb/IdMapper.h
            odb/OdbToRequest.cc
            odb/OdbToRequest.h
            odb/OdbMetadataDecoder.cc
            odb/OdbMetadataDecoder.h
            odb/OdbDecoder.cc
            odb/OdbContent.cc
            odb/OdbContent.h
            odb/OdbDecoder.h
            odb/OdbSplitter.cc
            odb/OdbSplitter.h
        )
    set( odc_libs odccore )
endif()

ecbuild_add_library(

    TARGET metkit

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/metkit

    PERSISTENT
        ${metkit_persistent_srcs}

    SOURCES
        ${metkit_srcs}

    PUBLIC_INCLUDES
       $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
       $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
       $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for metkit_version.h

    PRIVATE_LIBS
        "${odc_libs}"

    PUBLIC_LIBS
        eckit
        eckit_option
        ${grib_libs}
)
