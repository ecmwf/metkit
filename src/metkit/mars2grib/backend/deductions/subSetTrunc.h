/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */
#pragma once

#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the spectral subset truncation parameter (`subSetTrunc`).
 *
 * This deduction retrieves the optional parameter `subSetTrunc` from the
 * parameter dictionary (`par`). The value represents a truncation parameter
 * used to define a subset of spectral components to be retained or processed.
 *
 * If the key `subSetTrunc` is present in the parameter dictionary, its value
 * is retrieved and returned. If the key is not present, a default value of
 * `20` is used.
 *
 * The resolved value is always logged for diagnostic and traceability
 * purposes, regardless of whether it was provided explicitly or defaulted.
 *
 * Any error encountered during dictionary access is propagated using nested
 * exceptions to preserve full diagnostic context.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (unused by this deduction).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary, potentially containing the key
 *   `subSetTrunc`.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary (unused).
 *
 * @param[in] par
 *   Parameter dictionary from which `subSetTrunc` is retrieved.
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The resolved value of `subSetTrunc`. If not explicitly provided, the
 *   default value `20` is returned.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If an unexpected error occurs while accessing the parameter dictionary;
 *   the original exception is preserved via nested exceptions.
 *
 * @note
 *   No validation is currently performed on the resolved truncation value
 *   (e.g. range checks or consistency with the spectral grid). Such validation
 *   is expected to be handled at a higher level.
 *
 * @note
 *   This function follows a fail-fast strategy and uses nested exception
 *   propagation to ensure that error provenance is preserved across API
 *   boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
long resolve_SubSetTruncation_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        long subSetTrunc = get_opt<long>(par, "subSetTruncation").value_or(20);

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string logMsg = "subSetTrunc: looked up from Par dictionary with value: ";
            logMsg += std::to_string(subSetTrunc);
            return logMsg;
        }());

        return subSetTrunc;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get `subSetTrunc` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
