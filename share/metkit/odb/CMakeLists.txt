list( APPEND files_odb marsrequest.yaml class.table type.table stream.table group.txt )

foreach( _file ${files_odb} )

    configure_file( ${_file} ${CMAKE_BINARY_DIR}/share/metkit/odb/${_file} COPYONLY )

    if( HAVE_METKIT_CONFIG )
        install( FILES  ${CMAKE_BINARY_DIR}/share/metkit/odb/${_file}
                DESTINATION "share/metkit/odb"
                PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ )
    endif()
    
endforeach()

