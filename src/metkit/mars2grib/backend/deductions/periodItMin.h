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

#include <optional>
#include <string>

#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Retrieve the minimum wave period index (`iTmin`) from the parameter dictionary.
 *
 * This deduction attempts to extract the optional parameter `iTmin` from the
 * parameter dictionary (`par`). The value represents the minimum wave period
 * index used in wave spectral configurations.
 *
 * The parameter is treated as optional:
 * - if the key `iTmin` is present in the parameter dictionary, its value is
 *   returned wrapped in a `std::optional`;
 * - if the key is not present, an empty `std::optional` is returned.
 *
 * No validation of the retrieved value is currently performed. Any semantic
 * or physical validation (e.g. range checks, consistency with frequency or
 * period grids) is expected to be handled at a higher level.
 *
 * Any error encountered during dictionary access is propagated using nested
 * exceptions in order to preserve detailed diagnostic context.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (unused by this deduction).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary, potentially containing the key `iTmin`.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary (unused).
 *
 * @param[in] par
 *   Parameter dictionary from which the optional key `iTmin` is retrieved.
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   An optional containing the value of `iTmin` if present in the parameter
 *   dictionary; otherwise, an empty `std::optional`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If an unexpected error occurs while accessing the parameter dictionary;
 *   the original exception is preserved via nested exceptions.
 *
 * @note
 *   This function follows a fail-fast strategy with nested exception
 *   propagation and is intended for use at API or deduction boundaries.
 *
 * @todo
 *   Add validation of the retrieved `iTmin` value once the expected semantic
 *   constraints are fully specified.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<long> resolve_PeriodItMin_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the min frequency for wave period from par dictionary
        std::optional<long> itMinOpt = get_opt<long>(par, "iTmin");

        if (itMinOpt.has_value()) {
            // Logging of the waveDirectionGrid
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "itmin: looked up from Par dictionary with value: ";
                logMsg += std::to_string(itMinOpt.value());
                return logMsg;
            }());
        }
        else {
            // Logging of the waveDirectionGrid
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "itmin: not present in Par dictionary, no value retrieved";
                return logMsg;
            }());
        }

        return itMinOpt;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get period `iTmin` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
