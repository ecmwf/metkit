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
 * @brief Resolve the Laplacian operator coefficient from the parameter dictionary.
 *
 * This deduction retrieves the value associated with the key
 * `laplacianOperator` from the parameter dictionary (`par`). The value is
 * expected to be convertible to a `double` and is treated as mandatory.
 *
 * The Laplacian operator coefficient is typically used in spectral or
 * wave-model formulations to represent the action or scaling of the
 * Laplacian operator in a given physical or numerical context.
 *
 * If the key is missing or the value cannot be converted to the expected
 * type, a domain-specific exception is thrown. Any underlying error is
 * propagated using nested exceptions to preserve full diagnostic context.
 *
 * @tparam MarsDict_t
 *   Type of the MARS dictionary (unused by this deduction).
 *
 * @tparam ParDict_t
 *   Type of the parameter dictionary, expected to contain the key
 *   `laplacianOperator`.
 *
 * @tparam OptDict_t
 *   Type of the options dictionary (unused by this deduction).
 *
 * @param[in] mars
 *   MARS dictionary (unused).
 *
 * @param[in] par
 *   Parameter dictionary from which the Laplacian operator coefficient
 *   is retrieved.
 *
 * @param[in] opt
 *   Options dictionary (unused).
 *
 * @return
 *   The value of the Laplacian operator coefficient, returned as a `double`.
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *   If:
 *   - the key `laplacianOperator` is not present in the parameter dictionary,
 *   - the associated value cannot be converted to `double`,
 *   - any unexpected error occurs during dictionary access.
 *
 * @note
 *   This deduction assumes that the Laplacian operator coefficient is
 *   explicitly provided in the parameter dictionary and does not attempt
 *   to infer or default its value.
 *
 * @note
 *   The function follows a fail-fast strategy and uses nested exception
 *   propagation to ensure that error provenance is preserved across API
 *   boundaries.
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
auto resolve_LaplacianOperator_or_throw(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        return get_or_throw<double>(par, "laplacianOperator");
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Unable to get `laplacianOperator` from Par dictionary", Here()));
    };

    // Remove compiler warning
    __builtin_unreachable();
};

}  // namespace metkit::mars2grib::backend::deductions
