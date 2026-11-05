#pragma once

#include <iostream>
#include <string>

#include "metkit/codes/api/CodesAPI.h"

namespace metkit::grib2mars::rules::impl {

template <class MarsDict, class MiscDict, class OptDict_t>
void extractLegNumber(const std::string& keyword, const metkit::codes::CodesHandle& grib, MarsDict& mars,
                      MiscDict& misc, const OptDict_t& opts) {
    (void)grib;
    (void)mars;
    (void)misc;

    // std::cerr << "WARNING: ignoring unsupported grib2mars MARS keyword `" << keyword << "`" << std::endl;
}

}  // namespace metkit::grib2mars::rules::impl
