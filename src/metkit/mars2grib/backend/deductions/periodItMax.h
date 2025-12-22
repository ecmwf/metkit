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

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Retrieve the maximum wave period index (`iTmax`) from the parameter dictionary.
 *
 * This deduction attempts to extract the optional parameter `iTmax` from the
 * parameter dictionary (`par`). The value represents the maximum wave period
 * index used in wave spectral configurations.
 *
 * The parameter is treated as optional:
 * - if the key `iTmax` is present in the parameter dictionary, its value is
 *   returned wrapped in a `std::optional` and a diagnostic log message is emitted;
 * - if the key is not present, an empty `std::optional` is returned and this
 *   condition is explicitly logged.
 *
 * No validation of the retrieved value is currently performed. Any semantic
 * or physical validation (e.g. consistency with the frequency or period grid,
 * bounds checking, or monotonicity constraints) is expected to be handled at
 * a higher level.
 *
 * Any error encountered during dictionary access is propagated using nested
 * exceptions in order to preserve full diagnostic context.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (unused by this deduction).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary, potentially containing the key `iTmax`.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary (unused).
 *
 * @param[in] par
 *   Parameter dictionary from which the optional key `iTmax` is retrieved.
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   An optional containing the value of `iTmax` if present in the parameter
 *   dictionary; otherwise, an empty `std::optional`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If an unexpected error occurs while accessing the parameter dictionary;
 *   the original exception is preserved via nested exceptions.
 *
 * @note
 *   Diagnostic logging is emitted in both cases (value present or absent)
 *   to make the resolution behavior explicit in debug traces.
 *
 * @note
 *   This function follows a fail-fast strategy with nested exception
 *   propagation and is intended for use at API or deduction boundaries.
 *
 * @todo
 *   Add validation of the retrieved `iTmax` value once the expected semantic
 *   constraints are fully specified.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::optional<long> resolve_PeriodItMax_opt(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Get the min frequency for wave period from par dictionary
        std::optional<long> itMaxOpt = get_opt<long>(par, "iTmax");

        if (itMaxOpt.has_value()) {
            // Logging of the waveDirectionGrid
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "itmax: looked up from Par dictionary with value: ";
                logMsg += std::to_string(itMaxOpt.value());
                return logMsg;
            }());
        }
        else {
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "itmax: not present in Par dictionary, no value retrieved";
                return logMsg;
            }());
        }

        return itMaxOpt;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException("Unable to get period `iTmax` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
