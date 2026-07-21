# Standalone canonical ProductTimeSpec component.

set(product_time_spec_headers
    ProductTimeSpec.h
    ProductTimeSpecInput.h
    ProductTimeSpecResolver.h
)

set(product_time_spec_sources
    ProductTimeSpec.cc
)

ecbuild_add_library(
    TARGET metkit_mars2grib_product_time_spec
    INSTALL_HEADERS LISTED
    HEADER_DESTINATION ${INSTALL_INCLUDE_DIR}/metkit/mars2grib/productTimeSpec
    SOURCES
        ${product_time_spec_headers}
        ${product_time_spec_sources}
    PUBLIC_INCLUDES
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PUBLIC_LIBS
        eckit
)
