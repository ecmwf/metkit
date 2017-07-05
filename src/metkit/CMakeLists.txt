### config headers

ecbuild_generate_config_headers( DESTINATION ${INSTALL_INCLUDE_DIR}/metkit )

configure_file( metkit_config.h.in    metkit_config.h )
configure_file( metkit_version.h.in   metkit_version.h )
configure_file( metkit_version.cc.in  metkit_version.cc )

install(FILES
            ${CMAKE_CURRENT_BINARY_DIR}/metkit_config.h
            ${CMAKE_CURRENT_BINARY_DIR}/metkit_version.h
        DESTINATION
            ${INSTALL_INCLUDE_DIR}/metkit )

### metkit sources

list( APPEND metkit_srcs
metkit_version.cc
BaseProtocol.cc
BaseProtocol.h
ClientTask.cc
ClientTask.h
DHSProtocol.cc
DHSProtocol.h
MarsHandle.cc
MarsHandle.h
MarsLocation.cc
MarsLocation.h
MarsRequest.cc
MarsRequest.h
MarsExpension.cc
MarsExpension.h
MarsLanguage.cc
MarsLanguage.h
MarsParser.cc
MarsParser.h
MarsRequestHandle.cc
MarsRequestHandle.h
RequestEnvironment.cc
RequestEnvironment.h
config/LibMetkit.cc
config/LibMetkit.h
types/Type.cc
types/Type.h
types/TypeDate.cc
types/TypeDate.h
types/TypeAny.cc
types/TypeAny.h
types/TypeExpver.cc
types/TypeExpver.h
types/TypeToByList.cc
types/TypeToByList.h
types/TypeEnum.cc
types/TypeEnum.h
types/TypeParam.cc
types/TypeParam.h
types/TypeInteger.cc
types/TypeInteger.h
types/TypeRange.cc
types/TypeRange.h
types/TypeFloat.cc
types/TypeFloat.h
types/TypeTime.cc
types/TypeTime.h
types/TypeMixed.cc
types/TypeMixed.h
types/TypesFactory.cc
types/TypesFactory.h
)

if ( HAVE_GRIB )

    list( APPEND metkit_srcs
        grib/GribToRequest.cc
        grib/GribToRequest.h
        grib/EmosFile.h
        grib/EmosFile.cc
        grib/GribFile.h
        grib/GribFile.cc
        grib/GribAccessor.h
        grib/GribAccessor.cc
        grib/GribMutator.h
        grib/GribMutator.cc
        grib/GribIndex.h
        grib/GribIndex.cc
        grib/GribHandle.h
        grib/GribHandle.cc
        grib/GribDataBlob.h
        grib/GribDataBlob.cc
        grib/GribMetaData.h
        grib/GribMetaData.cc
        )

        set( grib_handling_pkg grib_api )
        if( HAVE_ECCODES )
          set( grib_handling_pkg eccodes )
        endif()

endif ()

if ( HAVE_NETCDF )

  list( APPEND metkit_srcs 
netcdf/NetCDFToRequest.h 
netcdf/Attribute.h
netcdf/CellMethodInputVariable.h
netcdf/CellMethodOutputVariable.h
netcdf/Codec.h
netcdf/CoordinateInputVariable.h
netcdf/CoordinateOutputVariable.h
netcdf/Counted.h
netcdf/DataInputVariable.h
netcdf/DataOutputVariable.h
netcdf/Date.h
netcdf/DateTime.h
netcdf/Dimension.h
netcdf/DummyInputVariable.h
netcdf/DummyMatrix.h
netcdf/DummyVariable.h
netcdf/Endowed.h
netcdf/Exceptions.h
netcdf/Field.h
netcdf/GregorianDateCodec.h
netcdf/HyperCube.h
netcdf/InputAttribute.h
netcdf/InputDimension.h
netcdf/InputField.h
netcdf/InputMatrix.h
netcdf/InputVariable.h
netcdf/Mapper.h
netcdf/Matrix.h
netcdf/MergeCoordinateMatrix.h
netcdf/MergeCoordinateStep.h
netcdf/MergeDataMatrix.h
netcdf/MergeDataStep.h
netcdf/MergePlan.h
netcdf/NCFile.h
netcdf/NCFileCache.h
netcdf/NetCDFToRequest.h
netcdf/OutputAttribute.h
netcdf/OutputDimension.h
netcdf/OutputField.h
netcdf/OutputVariable.h
netcdf/Remapping.h
netcdf/Reshape.h
netcdf/ReshapeVariableStep.h
netcdf/ScalarCoordinateInputVariable.h
netcdf/ScalarCoordinateOutputVariable.h
netcdf/Seconds.h
netcdf/SimpleInputVariable.h
netcdf/SimpleOutputVariable.h
netcdf/Step.h
netcdf/Time.h
netcdf/Tokenizer.h
netcdf/Translator.h
netcdf/Type.h
netcdf/UpdateCoordinateStep.h
netcdf/Value.h
netcdf/Variable.h
netcdf/VirtualInputDimension.h
netcdf/VirtualOutputDimension.h
netcdf/Attribute.cc
netcdf/CellMethodInputVariable.cc
netcdf/CellMethodOutputVariable.cc
netcdf/Codec.cc
netcdf/CoordinateInputVariable.cc
netcdf/CoordinateOutputVariable.cc
netcdf/Counted.cc
netcdf/DataInputVariable.cc
netcdf/DataOutputVariable.cc
netcdf/Date.cc
netcdf/DateTime.cc
netcdf/Dimension.cc
netcdf/DummyInputVariable.cc
netcdf/DummyMatrix.cc
netcdf/Endowed.cc
netcdf/Exceptions.cc
netcdf/Field.cc
netcdf/GregorianDateCodec.cc
netcdf/HyperCube.cc
netcdf/InputAttribute.cc
netcdf/InputDimension.cc
netcdf/InputField.cc
netcdf/InputMatrix.cc
netcdf/InputVariable.cc
netcdf/Matrix.cc
netcdf/MergeCoordinateMatrix.cc
netcdf/MergeCoordinateStep.cc
netcdf/MergeDataMatrix.cc
netcdf/MergeDataStep.cc
netcdf/MergePlan.cc
netcdf/NCFile.cc
netcdf/NCFileCache.cc
netcdf/OutputAttribute.cc
netcdf/OutputDimension.cc
netcdf/OutputField.cc
netcdf/OutputVariable.cc
netcdf/Remapping.cc
netcdf/Reshape.cc
netcdf/ReshapeVariableStep.cc
netcdf/ScalarCoordinateInputVariable.cc
netcdf/ScalarCoordinateOutputVariable.cc
netcdf/Seconds.cc
netcdf/SimpleInputVariable.cc
netcdf/SimpleOutputVariable.cc
netcdf/Step.cc
netcdf/Time.cc
netcdf/Tokenizer.cc
netcdf/Translator.cc
netcdf/Type.cc
netcdf/UpdateCoordinateStep.cc
netcdf/Value.cc
netcdf/Variable.cc
netcdf/VirtualInputDimension.cc
netcdf/VirtualOutputDimension.cc
#netcdf/ncmerge.cc
  )

  list( APPEND netcdflibs ${NETCDF_LIBRARIES} )

endif()

ecbuild_add_library(

    TARGET metkit

    INSTALL_HEADERS LISTED

    HEADER_DESTINATION
        ${INSTALL_INCLUDE_DIR}/metkit

    GENERATED
        metkit_version.cc

    SOURCES
        ${metkit_srcs}

    PRIVATE_INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${GRIB_API_INCLUDE_DIRS}
        ${NETCDF_INCLUDE_DIRS}

    LIBS
        eckit
        ${grib_handling_pkg}
        ${netcdflibs}
)
