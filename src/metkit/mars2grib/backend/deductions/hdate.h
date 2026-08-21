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
/// @file hdate.h
/// @brief Public deduction header for the optional raw MARS `hdate` key.
///
/// Exposes `resolve_Hdate_opt`, the canonical entry point that resolves the raw
/// optional MARS `hdate` source into a validated `eckit::Date`.
///
/// This deduction owns:
/// - direct `hdate` dictionary access;
/// - lexical parsing of the supported raw MARS `hdate` representations;
/// - local validation that the parsed fields form a valid calendar date.
///
/// This deduction does NOT:
/// - read any other temporal key;
/// - apply defaulting or cross-key validation;
/// - construct a `DateTime` or any higher-level temporal artifact.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <optional>
#include <sstream>
#include <string>

#include "eckit/types/Date.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/dateTimeHelpers.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the optional raw MARS `hdate` source.
///
/// @section Deduction contract
///   - Reads (MARS): `hdate`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `hdate` absent -> `std::nullopt`;
/// - numeric `hdate` -> parsed as `YYYYMMDD`;
/// - string `hdate` -> parsed as `YYYYMMDD` or `YYYY-MM-DD`;
/// - malformed, unsupported, or invalid values -> hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `hdate`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<eckit::Date>` containing the validated raw MARS
///         hindcast date when the key is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or invalid raw `hdate` input, with the
///         original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<eckit::Date> resolve_Hdate_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "hdate")) {
            return std::nullopt;
        }

        eckit::Date result{};
        if (auto value = get_opt<long>(mars, "hdate")) {
            result = detail::parseDateLong(*value, "hdate");
        }
        else if (auto value = get_opt<std::string>(mars, "hdate")) {
            result = detail::parseDateString(*value, "hdate");
        }
        else {
            throw Mars2GribDeductionException("Unsupported type for `hdate`", Here());
        }

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::ostringstream out;
            out << result;
            return std::string{"`hdate` resolved from input dictionaries: value='"} + out.str() + "'";
        }());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `hdate` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions
