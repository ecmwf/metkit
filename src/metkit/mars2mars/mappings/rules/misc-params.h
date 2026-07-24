/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file misc-params.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::rules::impl {


/// @brief Convert miscellaneous params
template <class InDict_t, class OutDict_t>
inline void convertMiscParams(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        const auto param = get_or_throw<long>(in, "param");

        switch (param) {
            case 151130:  // Practical salinity (psu to g/kg)
                set_or_throw<long>(out, "param", 262100);
                break;

            case 151131:  // Eastward surface sea water velocity (m/s)
                set_or_throw<long>(out, "param", 262140);
                break;

            case 151145:  // Sea surface height (m)
                set_or_throw<long>(out, "param", 262124);
                break;

            case 151148:  // Mixed layer depth (m)
                set_or_throw<long>(out, "param", 3067);
                break;

            case 151164:  // Average potential temperature in the upper 300m (deg' C to K)
                set_or_throw<long>(out, "param", 262144);
                misc.set("offsetValuesBy", 273.15);
                break;

            case 151175:  // Average sea water practical salinity in the upper 300m (psu to g/kg)
                set_or_throw<long>(out, "param", 262118);
                break;

            default:
                break;
        }
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to convert input dictionary in convertMiscParams", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl
