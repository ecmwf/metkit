#pragma once

#include <cstddef>

#include "metkit/mars2grib/backend/sections/initializers/section_initializer_0.h"
#include "metkit/mars2grib/backend/sections/initializers/section_initializer_1.h"
#include "metkit/mars2grib/backend/sections/initializers/section_initializer_2.h"
#include "metkit/mars2grib/backend/sections/initializers/section_initializer_3.h"
#include "metkit/mars2grib/backend/sections/initializers/section_initializer_4.h"
#include "metkit/mars2grib/backend/sections/initializers/section_initializer_5.h"
#include "metkit/mars2grib/backend/sections/initializers/section_initializer_core.h"

#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::initializers {

// ======================================================
// Section registries (sorted by TemplateNumber)
// ======================================================

template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
inline constexpr Entry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> Sec0Reg[] = {
    {0, &allocateTemplateNumber0<0, 0, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>}};

template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
inline constexpr Entry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> Sec1Reg[] = {
    {0, &allocateTemplateNumber1<1, 0, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>}};

template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
inline constexpr Entry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> Sec2Reg[] = {
    {1, &allocateTemplateNumber2<2, 1, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {15, &allocateTemplateNumber2<2, 15, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {24, &allocateTemplateNumber2<2, 24, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {36, &allocateTemplateNumber2<2, 36, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {1000, &allocateTemplateNumber2<2, 1000, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {1001, &allocateTemplateNumber2<2, 1001, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {1002, &allocateTemplateNumber2<2, 1002, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {1004, &allocateTemplateNumber2<2, 1004, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>}};

template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
inline constexpr Entry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> Sec3Reg[] = {
    {0, &allocateTemplateNumber3<3, 0, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {40, &allocateTemplateNumber3<3, 40, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {50, &allocateTemplateNumber3<3, 50, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {101, &allocateTemplateNumber3<3, 101, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {150, &allocateTemplateNumber3<3, 150, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>}};

template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
inline constexpr Entry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> Sec4Reg[] = {
    {0, &allocateTemplateNumber4<4, 0, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {1, &allocateTemplateNumber4<4, 1, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {2, &allocateTemplateNumber4<4, 2, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {8, &allocateTemplateNumber4<4, 8, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {11, &allocateTemplateNumber4<4, 11, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {12, &allocateTemplateNumber4<4, 12, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {32, &allocateTemplateNumber4<4, 32, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {33, &allocateTemplateNumber4<4, 33, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {40, &allocateTemplateNumber4<4, 40, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {41, &allocateTemplateNumber4<4, 41, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {42, &allocateTemplateNumber4<4, 42, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {43, &allocateTemplateNumber4<4, 43, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {50, &allocateTemplateNumber4<4, 50, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {45, &allocateTemplateNumber4<4, 45, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {46, &allocateTemplateNumber4<4, 46, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {85, &allocateTemplateNumber4<4, 85, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {48, &allocateTemplateNumber4<4, 48, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {49, &allocateTemplateNumber4<4, 49, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {60, &allocateTemplateNumber4<4, 60, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {61, &allocateTemplateNumber4<4, 61, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {76, &allocateTemplateNumber4<4, 76, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {77, &allocateTemplateNumber4<4, 77, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {78, &allocateTemplateNumber4<4, 78, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {79, &allocateTemplateNumber4<4, 79, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {99, &allocateTemplateNumber4<4, 99, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {100, &allocateTemplateNumber4<4, 100, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {103, &allocateTemplateNumber4<4, 103, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {104, &allocateTemplateNumber4<4, 104, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {142, &allocateTemplateNumber4<4, 142, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {143, &allocateTemplateNumber4<4, 143, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>}};

template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
inline constexpr Entry<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> Sec5Reg[] = {
    {0, &allocateTemplateNumber5<5, 0, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {42, &allocateTemplateNumber5<5, 42, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>},
    {51, &allocateTemplateNumber5<5, 51, MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>}};

// ======================================================
// Generic lookup (array size deduced)
// ======================================================
template <class EntryT, std::size_t N>
constexpr auto lookup(const EntryT (&table)[N], std::size_t templ) -> decltype(table[0].second) {
    for (std::size_t i = 0; i < N; ++i) {
        if (table[i].first == templ)
            return table[i].second;
    }
    return nullptr;
}

// ======================================================
// Section dispatch
// ======================================================
template <class MarsDict_t, class GeoDict_t, class ParDict_t, class OptDict_t, class OutDict_t>
Fn<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t> getSectionInitializerFn(std::size_t section,
                                                                                   std::size_t templ) {
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    try {
        switch (section) {
            case 0:
                return lookup(Sec0Reg<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, templ);
            case 1:
                return lookup(Sec1Reg<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, templ);
            case 2:
                return lookup(Sec2Reg<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, templ);
            case 3:
                return lookup(Sec3Reg<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, templ);
            case 4:
                return lookup(Sec4Reg<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, templ);
            case 5:
                return lookup(Sec5Reg<MarsDict_t, GeoDict_t, ParDict_t, OptDict_t, OutDict_t>, templ);
            default:
                return nullptr;
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribGenericException("Error getting section initializer function for section " +
                                                             std::to_string(section) + " template " +
                                                             std::to_string(templ),
                                                         Here()));
    }
}

}  // namespace metkit::mars2grib::backend::sections::initializers
