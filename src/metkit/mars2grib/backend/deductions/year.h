/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file year.h
/// @brief Public deduction header for the optional raw MARS `year` key.
///
/// Exposes `resolve_Year_opt`, the canonical entry point that resolves the raw
/// optional MARS `year` source into a normalized integer value.
///
/// This deduction owns:
/// - direct `year` dictionary access;
/// - lexical parsing of integer and string integer representations.
///
/// This deduction does NOT:
/// - validate the relationship between `year` and `month`;
/// - construct a date or datetime;
/// - apply any ProductTimeSpec-specific semantics.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <string>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the optional raw MARS `year` source.
///
/// @section Deduction contract
///   - Reads (MARS): `year`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `year` absent -> `std::nullopt`;
/// - numeric `year` -> returned verbatim;
/// - string `year` -> parsed as one complete decimal integer;
/// - malformed or unsupported values -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `year`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<long>` containing the normalized raw MARS year when
///         the key is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed or unsupported raw `year` input, with the original
///         cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<long> resolve_Year_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "year")) {
            return std::nullopt;
        }

        long result = 0;
        if (auto value = get_opt<long>(mars, "year")) {
            result = *value;
        }
        else if (auto value = get_opt<std::string>(mars, "year")) {
            result = detail::parseLongStrict(*value, "year");
        }
        else {
            throw Mars2GribDeductionException("Unsupported type for `year`", Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            return std::string{"`year` resolved from input dictionaries: value='"} + std::to_string(result) + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException("Failed to resolve `year` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions
