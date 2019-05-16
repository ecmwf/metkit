list( APPEND files language.yaml paramids.yaml params.yaml wind )

foreach( _file ${files} )

    configure_file( ${_file} ${CMAKE_BINARY_DIR}/share/metkit/${_file} COPYONLY )

    install( FILES  ${CMAKE_BINARY_DIR}/share/metkit/${_file}
             DESTINATION "share/metkit"
             PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )

endforeach()

add_subdirectory( odb )
