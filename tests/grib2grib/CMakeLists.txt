ecbuild_add_executable(
    TARGET  roundtrip
    SOURCES roundtrip.cc
    LIBS    metkit
    NOINSTALL
)

macro(add_roundtrip_tests test grid)
    set(target metkit_test_grib2grib_${test})

    ecbuild_add_test(
        TARGET            "${target}"
        COMMAND           roundtrip
        ARGS              "${test}.grib2"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        ENVIRONMENT       ECCODES_ECKIT_GEO=1
    )

    ecbuild_add_test(
        TARGET            "${target}_${grid}"
        COMMAND           roundtrip
        ARGS              "${test}.grib2" ${grid}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        ENVIRONMENT       ECCODES_ECKIT_GEO=1
    )
endmacro()

add_roundtrip_tests("mtg2-sfc" "O1280")
add_roundtrip_tests("mtg2-o2d" "eORCA025_T")
# add_roundtrip_tests("mtg2-t" "T20")
# add_roundtrip_tests("mtg2-vod" "T20")
