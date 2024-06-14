list( APPEND files_mars2grib RuleList.yaml )

foreach( _file ${files_mars2grib} )

    configure_file( ${_file} ${CMAKE_BINARY_DIR}/share/metkit/mars2grib/${_file} COPYONLY )

    install( FILES  ${CMAKE_BINARY_DIR}/share/metkit/mars2grib/${_file}
             DESTINATION "share/metkit/mars2grib"
             PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endforeach()

add_subdirectory( rules )
