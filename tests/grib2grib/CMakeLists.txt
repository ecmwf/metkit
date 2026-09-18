ecbuild_add_executable(
    TARGET  roundtrip
    SOURCES roundtrip.cc
    LIBS    metkit eccodes
    NOINSTALL
)

macro(add_roundtrip_test arg1)
    string(JOIN "_" _extra ${ARGN})
    string(MAKE_C_IDENTIFIER "metkit_test_grib2grib_${arg1}_${_extra}" _target)
    string(REGEX REPLACE "__+" "_" _target "${_target}")
    string(REGEX REPLACE "_$" "" _target "${_target}")

    ecbuild_add_test(
        TARGET            "${_target}"
        COMMAND           roundtrip
        ARGS              "${arg1}" ${ARGN}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        ENVIRONMENT       ECCODES_ECKIT_GEO=1
    )
endmacro()

add_roundtrip_test("mtg2-sfc.grib2")
add_roundtrip_test("mtg2-sfc.grib2" "--grid=O1280")
add_roundtrip_test("mtg2-o2d.grib2")
add_roundtrip_test("mtg2-o2d.grib2" "--grid=eORCA025_T")
add_roundtrip_test("mtg2-t.grib2")
add_roundtrip_test("mtg2-t.grib2" "--grid=T20")
add_roundtrip_test("mtg2-vod.grib2")
add_roundtrip_test("mtg2-vod.grib2" "--grid=T20")
