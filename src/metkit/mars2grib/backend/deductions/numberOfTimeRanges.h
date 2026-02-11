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
/// @file numberOfTimeRanges.h
/// @brief Deduction of the GRIB `numberOfTimeRanges` key.
///
/// This header defines deduction utilities used by the mars2grib backend
/// to resolve the **number of time ranges** associated with statistical
/// processing.
///
/// The deduction determines the number of time ranges based on the
/// presence and structure of MARS statistical metadata.
///
/// In particular:
/// - the MARS key `timespan` is mandatory
/// - the MARS key `stattype` is used to determine the number of
/// statistical blocks when present
///
/// Error handling follows a strict fail-fast strategy:
/// - missing or invalid inputs cause immediate failure
/// - errors are reported using domain-specific deduction exceptions
/// - original errors are preserved via nested exception propagation
///
/// Logging follows the mars2grib deduction policy:
/// - this deduction performs no logging
///
/// @section References
/// Concept:
/// - @ref statisticsEncoding.h
///
/// Related deductions:
/// - @ref timeSpanInSeconds.h
/// - @ref timeIncrementInSeconds.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>

// Details
#include "metkit/mars2grib/backend/deductions/detail/timeUtils.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the number of time ranges for statistical processing.
///
/// @section Deduction contract
/// - Reads: `mars["timespan"]`, optionally `mars["stattype"]`
/// - Writes: none
/// - Side effects: none
/// - Failure mode: throws
///
/// This deduction computes the number of time ranges required by GRIB
/// statistical processing templates.
///
/// Resolution rules:
/// - if `timespan` is missing → failure
/// - if `stattype` is missing → returns `1`
/// - otherwise:
/// - the number of time ranges is computed as
/// `countBlocks(stattype) + 1`
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary. Must support access to `timespan`
/// and optionally `stattype`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (unused by this deduction).
///
/// @param[in] mars
/// MARS dictionary providing statistical metadata.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @return
/// The number of time ranges.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If `timespan` is missing or if any unexpected error occurs during
/// deduction.
///
/// @note
/// This deduction is deterministic and does not rely on pre-existing
/// GRIB header state.
///
template <class MarsDict_t, class ParDict_t>
long numberOfTimeRanges(const MarsDict_t& mars, const ParDict_t& par) {

    using metkit::mars2grib::backend::deductions::detail::countBlocks;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the mars.levelist
        bool hasTimespan = has(mars, "timespan");
        bool hasStatType = has(mars, "stattype");

        // Error if timespan is missing
        if (!hasTimespan) {
            throw Mars2GribDeductionException("`timespan` is required to compute number of time ranges", Here());
        }

        // Handle trivial case
        if (hasTimespan || !hasStatType) {

            // Retrieve number Of Timeranges
            long numberOfTimeRanges = 1;

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`numberOfTimeRanges` resolved from input dictionaries: value='";
                logMsg += std::to_string(numberOfTimeRanges) + "'";
                return logMsg;
            }());

            return numberOfTimeRanges;
        }

        if (hasStatType) {

            // Retrieve MARS stattype
            std::string statTypeVal = get_or_throw<std::string>(mars, "stattype");

            // Count number of blocks in stattype
            long numberOfBlocks = static_cast<long>(countBlocks(statTypeVal));

            // Compute number of time ranges
            long numberOfTimeRanges = numberOfBlocks + 1;

            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`numberOfTimeRanges` resolved from input dictionaries: value='";
                logMsg += std::to_string(numberOfTimeRanges) + "'";
                return logMsg;
            }());

            // Number of time ranges = number of blocks + 1
            return numberOfTimeRanges;
        }

        // Remove compiler warning
        __builtin_unreachable();
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `numberOfTimeRanges` from input dictionaries", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
