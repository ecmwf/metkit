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
/// @file time.h
/// @brief Public deduction header for the optional raw MARS `time` key.
///
/// Exposes `resolve_Time_opt`, the canonical entry point that resolves the raw
/// optional MARS `time` source into a validated `eckit::Time`.
///
/// This deduction owns:
/// - direct `time` dictionary access;
/// - lexical parsing of the supported raw MARS `time` representations;
/// - local validation of compact and colon-separated hour/minute forms.
///
/// This deduction does NOT:
/// - read or validate the raw MARS `date` key;
/// - apply defaulting;
/// - construct a `DateTime` or enforce cross-key temporal invariants.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <sstream>
#include <string>

#include "eckit/types/Time.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/dateTimeHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the optional raw MARS `time` source.
///
/// @section Deduction contract
///   - Reads (MARS): `time`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `time` absent -> `std::nullopt`;
/// - numeric `time` -> parsed as compact MARS `HHMM`;
/// - string `time` -> parsed as `HHMM` or `HH:MM`;
/// - malformed, unsupported, or invalid values -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `time`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<eckit::Time>` containing the validated raw MARS time
///         when the key is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or invalid raw `time` input, with the
///         original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<eckit::Time> resolve_Time_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "time")) {
            return std::nullopt;
        }

        eckit::Time result{};
        if (auto value = get_opt<long>(mars, "time")) {
            result = detail::parseTimeLong(*value, "time");
        }
        else if (auto value = get_opt<std::string>(mars, "time")) {
            result = detail::parseTimeString(*value, "time");
        }
        else {
            throw Mars2GribDeductionException("Unsupported type for `time`", Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::ostringstream out;
            out << result;
            return std::string{"`time` resolved from input dictionaries: value='"} + out.str() + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException("Failed to resolve `time` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions
