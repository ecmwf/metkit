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

/// @file shiftOfTails.h
/// @brief Conversion rules used by the mars2mars mapper.
#pragma once

#include <string>
#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::rules::impl {


/// @brief Convert the shift-of-tails percentile from MARS `number` to `quantile`
///
/// In GRIB1 (local definition 19) the SOT percentile is MARS `number` (90 or 10), with the outer percentile in
/// `efiOrder` (99 or 1). In GRIB2 the two percentiles are reference-period parameters, and ecCodes exposes them as
/// MARS `quantile` (ECC-2000): SOT90 is `90-99:100`, SOT10 is `1-10:100`.
template <class InDict_t, class OutDict_t, class OptDict_t>
inline void convertShiftOfTails(const InDict_t& in, OutDict_t& out, eckit::LocalConfiguration& misc,
                                const OptDict_t& opts) {

    using metkit::mars2mars::utils::dict_traits::get_or_throw;
    using metkit::mars2mars::utils::dict_traits::has;
    using metkit::mars2mars::utils::dict_traits::set_or_throw;
    using metkit::mars2mars::utils::dict_traits::setMissing_or_throw;
    using metkit::mars2mars::utils::exceptions::Mars2marsGenericException;

    try {
        (void)misc;
        (void)opts;
        if (get_or_throw<std::string>(in, "type") != "sot" || !has(in, "number")) {
            return;
        }

        const long number = get_or_throw<long>(in, "number");
        if (number == 90) {
            set_or_throw<std::string>(out, "quantile", "90-99:100");
        }
        else if (number == 10) {
            set_or_throw<std::string>(out, "quantile", "1-10:100");
        }
        else {
            throw Mars2marsGenericException(
                "Unsupported shift-of-tails `number` " + std::to_string(number) + " (expected 10 or 90)", Here());
        }
        setMissing_or_throw(out, "number");
    }
    catch (...) {
        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2marsGenericException("Failed to convert input dictionary in convertShiftOfTails", Here()));
    }
}

}  // namespace metkit::mars2mars::rules::impl
