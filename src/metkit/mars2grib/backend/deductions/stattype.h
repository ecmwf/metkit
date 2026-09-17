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
/// @file stattype.h
/// @brief Public deduction header for `stattype`.
///
/// Exposes `resolve_Stattype_opt` and `resolve_Stattype_or_throw`, the
/// canonical entry points that resolve the optional parsed MARS `stattype`
/// source from input dictionaries.
///
/// This deduction owns:
/// - direct `stattype` dictionary access;
/// - whitelist validation of the currently supported MARS `stattype` language;
/// - parsing of each compact block into one time range and one statistical
///   processing type;
/// - preservation of parsed block order.
///
/// This deduction does NOT:
/// - combine `stattype` with `timespan` or caller-supplied inner processing;
/// - classify ProductTimeSpec semantics;
/// - construct the final ProductTimeSpec model.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include "metkit/mars2grib/utils/profiling/Profiling.h"

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/common.h"
#include "metkit/mars2grib/backend/deductions/detail/parseHelpers.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions {

struct ParsedStatTypeBlock00 {
    TimeDuration timeRange{};
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing{tables::TypeOfStatisticalProcessing::Missing};
};

using ParsedStatTypeBlocks = std::vector<ParsedStatTypeBlock00>;

namespace detail {

///
/// @brief Test whether one lowercase `stattype` token is whitelisted.
///
/// @param[in] value Lowercase `stattype` token.
/// @return `true` when the token belongs to the currently supported MARS
///         whitelist.
///
template <class Cntx_t>
inline bool isWhitelistedStatType(const std::string& value, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    constexpr std::array<std::string_view, 25> whitelist{
        "moav",      "momn",      "momx",      "mosd",      "daac",      "daav",      "damn",
        "damx",      "dasd",      "moav_daav", "moav_damn", "moav_damx", "moav_dasd", "momn_daav",
        "momn_damn", "momn_damx", "momn_dasd", "momx_daav", "momx_damn", "momx_damx", "momx_dasd",
        "mosd_daav", "mosd_damn", "mosd_damx", "mosd_dasd"};
    {
        bool result = std::find(whitelist.begin(), whitelist.end(), value) != whitelist.end();
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
}

///
/// @brief Parse one compact `stattype` operation suffix.
///
/// @param[in] operation Two-character operation code.
/// @return Corresponding GRIB statistical processing type.
/// @throws Mars2GribDeductionException if the operation code is unsupported.
///
template <class Cntx_t>
inline tables::TypeOfStatisticalProcessing parseStatOperation(const std::string& operation, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (operation == "av") {
        {
            tables::TypeOfStatisticalProcessing result = tables::TypeOfStatisticalProcessing::Average;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (operation == "ac") {
        {
            tables::TypeOfStatisticalProcessing result = tables::TypeOfStatisticalProcessing::Accumulation;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (operation == "mn") {
        {
            tables::TypeOfStatisticalProcessing result = tables::TypeOfStatisticalProcessing::Minimum;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (operation == "mx") {
        {
            tables::TypeOfStatisticalProcessing result = tables::TypeOfStatisticalProcessing::Maximum;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }
    if (operation == "sd") {
        {
            tables::TypeOfStatisticalProcessing result = tables::TypeOfStatisticalProcessing::StandardDeviation;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    throw Mars2GribDeductionException("Unsupported stattype operation: '" + operation + "'", Here());
}

///
/// @brief Parse one complete compact `stattype` value.
///
/// The input is validated against the supported whitelist first, then split into
/// one or more underscore-separated four-character blocks. Each block yields one
/// parsed time range and one statistical processing type.
///
/// @param[in] raw Raw `stattype` string from MARS.
/// @return Parsed blocks in stored textual order.
/// @throws Mars2GribDeductionException if the value is malformed, unsupported,
///         or not in the supported whitelist.
///
template <class Cntx_t>
inline ParsedStatTypeBlocks parseStattypeValue(const std::string& raw, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    const std::string value = lower(raw, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
    if (!isWhitelistedStatType(value, metkit::mars2grib::utils::profiling::callSite(cntx, Here()))) {
        throw Mars2GribDeductionException("`stattype` is not in the supported MARS whitelist: '" + raw + "'", Here());
    }

    ParsedStatTypeBlocks result;
    std::size_t start = 0;
    while (start < value.size()) {
        const std::size_t end   = value.find('_', start);
        const std::string block = value.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (block.size() != 4) {
            throw Mars2GribDeductionException("Invalid stattype block: '" + block + "'", Here());
        }

        const std::string period    = block.substr(0, 2);
        const std::string operation = block.substr(2, 2);

        TimeDuration range;
        if (period == "mo") {
            range = TimeDuration{1, tables::TimeUnit::Month};
        }
        else if (period == "da") {
            range = TimeDuration{1, tables::TimeUnit::Day};
        }
        else {
            throw Mars2GribDeductionException("Unsupported stattype period: '" + period + "'", Here());
        }

        result.push_back(ParsedStatTypeBlock00{range, parseStatOperation(operation, metkit::mars2grib::utils::profiling::callSite(cntx, Here()))});
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }

    metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace detail

///
/// @brief Resolve `stattype` as an optional parsed block sequence.
///
/// @section Deduction contract
///   - Reads (MARS): `stattype`
///   - Reads (par):  none (signature-only, reserved)
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// Resolution rules:
/// - `stattype` absent -> `std::nullopt`;
/// - `stattype` present -> parse one or more compact blocks in textual order;
/// - unsupported whitelist entries, periods, operations, or malformed blocks ->
///   hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] mars  MARS dictionary providing `stattype`.
/// @param[in] par   Parameter dictionary (signature-only).
/// @param[in] opt   Options dictionary (signature-only).
///
/// @return `std::optional<ParsedStatTypeBlocks>` containing the parsed block
///         sequence when `stattype` is present, `std::nullopt` otherwise.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on malformed, unsupported, or non-whitelisted `stattype` input, with
///         the original cause attached via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class Cntx_t>
std::optional<ParsedStatTypeBlocks> resolve_Stattype_opt(const MarsDict_t& mars, const ParDict_t& par,
                                                         const OptDict_t& opt, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    (void)par;
    (void)opt;

    try {
        if (!has(mars, "stattype", metkit::mars2grib::utils::profiling::callSite(cntx, Here()))) {
            {
                std::optional<ParsedStatTypeBlocks> result = std::nullopt;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }
        if (auto value = get_opt<std::string>(mars, "stattype", metkit::mars2grib::utils::profiling::callSite(cntx, Here()))) {
            ParsedStatTypeBlocks result =
                detail::parseStattypeValue(*value, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            MARS2GRIB_LOG_RESOLVE([&]() {
                return std::string{"`stattype` resolved from input dictionaries: blocks='"} +
                       std::to_string(result.size()) + "'";
            }());
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        throw Mars2GribDeductionException("`stattype` must be a string", Here());
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `stattype` from input dictionaries", Here()));
    }

    mars2gribUnreachable();
}

///
/// @brief Resolve `stattype` or throw if absent.
///
/// Thin wrapper around `resolve_Stattype_opt` that converts `std::nullopt` into
/// a hard error.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type.
/// @tparam OptDict_t    Options dictionary type.
///
/// @param[in] mars  MARS dictionary providing `stattype`.
/// @param[in] par   Parameter dictionary (forwarded).
/// @param[in] opt   Options dictionary (forwarded).
///
/// @return The parsed `stattype` block sequence.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         if the source is absent, malformed, unsupported, or non-whitelisted;
///         failures are wrapped via `std::throw_with_nested`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t, class Cntx_t>
ParsedStatTypeBlocks resolve_Stattype_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {
        const std::optional<ParsedStatTypeBlocks> resolved =
            resolve_Stattype_opt(mars, par, opt, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
        if (resolved.has_value()) {
            {
                ParsedStatTypeBlocks result = *resolved;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }
        else {
            {
                ParsedStatTypeBlocks result = ParsedStatTypeBlocks{};
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }
    }
    catch (...) {
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `stattype` from Mars dictionary", Here()));
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions
