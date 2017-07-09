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
netcdf/Attribute.cc
netcdf/Attribute.h
netcdf/CellMethodInputVariable.cc
netcdf/CellMethodInputVariable.h
netcdf/CellMethodOutputVariable.cc
netcdf/CellMethodOutputVariable.h
netcdf/Codec.cc
netcdf/Codec.h
netcdf/CoordinateInputVariable.cc
netcdf/CoordinateInputVariable.h
netcdf/CoordinateOutputVariable.cc
netcdf/CoordinateOutputVariable.h
netcdf/DataInputVariable.cc
netcdf/DataInputVariable.h
netcdf/DataOutputVariable.cc
netcdf/DataOutputVariable.h
netcdf/Dimension.cc
netcdf/Dimension.h
netcdf/DummyInputVariable.cc
netcdf/DummyInputVariable.h
netcdf/DummyMatrix.cc
netcdf/DummyMatrix.h
netcdf/DummyVariable.h
netcdf/Endowed.cc
netcdf/Endowed.h
netcdf/Exceptions.cc
netcdf/Exceptions.h
netcdf/Dataset.cc
netcdf/Dataset.h
netcdf/GregorianDateCodec.cc
netcdf/GregorianDateCodec.h
netcdf/HyperCube.cc
netcdf/HyperCube.h
netcdf/InputAttribute.cc
netcdf/InputAttribute.h
netcdf/InputDimension.cc
netcdf/InputDimension.h
netcdf/InputDataset.cc
netcdf/InputDataset.h
netcdf/InputMatrix.cc
netcdf/InputMatrix.h
netcdf/InputVariable.cc
netcdf/InputVariable.h
netcdf/Mapper.h
netcdf/Matrix.cc
netcdf/Matrix.h
netcdf/MergeCoordinateMatrix.cc
netcdf/MergeCoordinateMatrix.h
netcdf/MergeCoordinateStep.cc
netcdf/MergeCoordinateStep.h
netcdf/MergeDataMatrix.cc
netcdf/MergeDataMatrix.h
netcdf/MergeDataStep.cc
netcdf/MergeDataStep.h
netcdf/MergePlan.cc
netcdf/MergePlan.h
netcdf/NCFile.cc
netcdf/NCFile.h
netcdf/NCFileCache.cc
netcdf/NCFileCache.h
netcdf/OutputAttribute.cc
netcdf/OutputAttribute.h
netcdf/OutputDimension.cc
netcdf/OutputDimension.h
netcdf/OutputDataset.cc
netcdf/OutputDataset.h
netcdf/OutputVariable.cc
netcdf/OutputVariable.h
netcdf/Remapping.cc
netcdf/Remapping.h
netcdf/Reshape.cc
netcdf/Reshape.h
netcdf/GridSpec.h
netcdf/GridSpec.cc
netcdf/RegularLL.h
netcdf/RegularLL.cc
netcdf/Field.h
netcdf/Field.cc
netcdf/InputField.h
netcdf/InputField.cc
netcdf/ReshapeVariableStep.cc
netcdf/ReshapeVariableStep.h
netcdf/ScalarCoordinateInputVariable.cc
netcdf/ScalarCoordinateInputVariable.h
netcdf/ScalarCoordinateOutputVariable.cc
netcdf/ScalarCoordinateOutputVariable.h
netcdf/SimpleInputVariable.cc
netcdf/SimpleInputVariable.h
netcdf/SimpleOutputVariable.cc
netcdf/SimpleOutputVariable.h
netcdf/Step.cc
netcdf/Step.h
netcdf/Type.cc
netcdf/Type.h
netcdf/UpdateCoordinateStep.cc
netcdf/UpdateCoordinateStep.h
netcdf/Value.cc
netcdf/Value.h
netcdf/Variable.cc
netcdf/Variable.h
netcdf/VirtualInputDimension.cc
netcdf/VirtualInputDimension.h
netcdf/VirtualOutputDimension.cc
netcdf/VirtualOutputDimension.h
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
