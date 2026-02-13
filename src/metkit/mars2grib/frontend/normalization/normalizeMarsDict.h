/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file normalization.h
/// @brief Orchestration of input dictionary sanitization.
///
#pragma once

// System includes
#include <regex>
#include <string>

// Project includes
#include "eckit/value/Value.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/enableOptions.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::normalization {


namespace hack {

///
/// @brief Normalizes legacy MARS grid strings in-place.
/// @return true if the dictionary was actually modified.
///
template <class MarsDict_t>
bool needFixMarsGrid(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;

    auto marsGrid = get_opt<std::string>(mars, "grid");
    if (!marsGrid)
        return false;

    static const std::regex pattern{R"(L(\d+)x(\d+))"};
    return std::regex_match(*marsGrid, pattern);
}


template <class MarsDict_t>
bool fixMarsGrid(MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_opt;

    auto marsGrid = get_opt<std::string>(mars, "grid");
    if (!marsGrid)
        return false;

    // Matches legacy LxN format (e.g., L640x320)
    static const std::regex pattern{R"(L(\d+)x(\d+))"};
    std::smatch match;

    if (std::regex_match(*marsGrid, match, pattern)) {
        const long ni         = std::stol(match[1].str());
        const long nj         = std::stol(match[2].str());
        const double deltaLon = 360.0 / static_cast<double>(ni);
        const double deltaLat = 180.0 / static_cast<double>(nj - 1);

        mars.set("grid", std::to_string(deltaLon) + "/" + std::to_string(deltaLat));
        return true;
    }

    return false;
}

}  // namespace hack

///
/// @brief Conditionally sanitizes a dictionary by running a suite of atomic key checks.
///
/// This function iterates through specific keys (truncation, grid, etc.) and
/// applies the normalization rules defined in the @ref per_key directory.
///
/// @tparam MiscDict_t Type of the dictionary to be sanitized
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in]  mars     Original dictionary
/// @param[in]  opt      Options driving the sanitization policy
/// @param[out] scratch  Buffer to store results if a transformation occurs
///
/// @todo build structure around tests
///
/// @return A const reference to the sanitized or original dictionary
///
template <class MarsDict_t, class OptDict_t>
const MarsDict_t& normalize_MarsDict_if_enabled(const MarsDict_t& mars, const OptDict_t& opt,
                                                const eckit::Value& language, MarsDict_t& scratch) {

    using metkit::mars2grib::utils::fixMarsGridEnabled;
    using metkit::mars2grib::utils::normalizeMarsEnabled;
    using metkit::mars2grib::utils::dict_traits::get_opt;

    // Track if we have moved data into scratch yet
    bool modified = false;

    bool needsFix      = fixMarsGridEnabled(opt) && hack::needFixMarsGrid(mars);
    bool needsSanitize = normalizeMarsEnabled(opt);

    if (needsFix || needsSanitize) {
        // We pay the performance debt here for the user's convenience
        scratch = mars;

        if (needsSanitize) {
            // Future placeholder for language-based logic
        }

        if (needsFix) {
            hack::fixMarsGrid(scratch);
        }

        return scratch;
    }

    // No flags enabled: zero overhead, return original reference
    return mars;
}

}  // namespace metkit::mars2grib::frontend::normalization