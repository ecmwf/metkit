ecbuild_add_executable(

    TARGET netcdf4-example

    CONDITION HAVE_NETCDF

    SOURCES
        netcdf4-example.cc

    INCLUDES
        ${ECKIT_INCLUDE_DIRS}
        ${NETCDF_INCLUDE_DIRS}

    LIBS
        eckit
        ${NETCDF_LIBRARIES}
)
