/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file typeOfProcessedData.h
 * @brief Deduction of the GRIB `typeOfProcessedData` identifier.
 *
 * This header defines the deduction responsible for resolving the
 * GRIB `typeOfProcessedData` key (GRIB2 Code Table 4.10).
 *
 * The value may be explicitly provided via parametrization or,
 * if absent, deterministically deduced from MARS metadata.
 *
 * Deductions:
 * - extract values from input dictionaries
 * - apply deterministic resolution logic
 * - emit structured diagnostic logging
 *
 * Deductions do NOT:
 * - infer missing values beyond documented rules
 * - guess semantics not defined by policy
 * - apply implicit defaults without explicit mapping
 *
 * Error handling follows a strict fail-fast strategy with nested
 * exception propagation to preserve full diagnostic context.
 *
 * Logging policy:
 * - OVERRIDE: explicit value provided via parameter dictionary
 * - RESOLVE: value deduced from MARS metadata
 *
 * @section References
 * Concept:
 *   - @ref dataTypeEncoding.h
 *
 * Related deductions:
 *   - @ref productionStatusOfProcessedData.h
 *
 * @ingroup mars2grib_backend_deductions
 */
#pragma once

// System includes
#include <string>

// Tables includes
#include "metkit/mars2grib/backend/tables/typeOfProcessedData.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::deductions {

/**
 * @brief Resolve the GRIB `typeOfProcessedData` key.
 *
 * This deduction determines the value of the GRIB
 * `typeOfProcessedData` key using the following precedence:
 *
 * 1. **User override (parameter dictionary)**
 *    If the key `typeOfProcessedData` is present in the parameter
 *    dictionary, its value is taken as authoritative. The value may
 *    be provided either as:
 *    - a numeric GRIB code (`long`)
 *    - a symbolic GRIB name (`string`)
 *
 * 2. **Automatic deduction (MARS dictionary)**
 *    If no override is provided, the value is deduced from
 *    `mars::type` using a fixed, explicitly defined mapping.
 *
 * Unsupported or unmapped MARS types result in the value
 * `TypeOfProcessedData::Missing`.
 *
 * @tparam MarsDict_t Type of the MARS dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary (unused)
 *
 * @param[in] mars MARS dictionary providing `class`, `type`, and `stream`
 * @param[in] par  Parameter dictionary providing optional override
 * @param[in] opt  Options dictionary (unused)
 *
 * @return The resolved `TypeOfProcessedData` enumeration value
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
 *         If:
 *         - override value is present but invalid
 *         - dictionary access fails
 *         - any unexpected error occurs during deduction
 */
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::TypeOfProcessedData resolve_TypeOfProcessedData_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                 const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // Retrieve mandatory class from MARS dictionary
        std::string marsClass = get_or_throw<std::string>(mars, "class");

        // Retrieve mandatory type from MARS dictionary
        std::string marsType = get_or_throw<std::string>(mars, "type");

        // Retrieve mandatory stream from MARS dictionary
        std::string marsStream = get_or_throw<std::string>(mars, "stream");

        // Return value
        tables::TypeOfProcessedData result = tables::TypeOfProcessedData::Missing;

        if (has(par, "typeOfProcessedData")) {

            // Retrieve mandatory override from parameter dictionary
            if (has<long>(par, "typeOfProcessedData")) {
                long typeOfProcessedDataVal = get_or_throw<long>(par, "typeOfProcessedData");

                // Convert long to enum (validate)
                result = tables::long2enum_TypeOfProcessedData_or_throw(typeOfProcessedDataVal);
            }
            else if (has<std::string>(par, "typeOfProcessedData")) {
                std::string typeOfProcessedDataVal = get_or_throw<std::string>(par, "typeOfProcessedData");

                // Convert string to enum (validate)
                result = tables::name2enum_TypeOfProcessedData_or_throw(typeOfProcessedDataVal);
            }
            else {
                throw Mars2GribDeductionException(
                    "Key `typeOfProcessedData` is not of expected type `long` or `string`", Here());
            }

            // Emit OVERRIDE log entry
            MARS2GRIB_LOG_OVERRIDE([&]() {
                std::string logMsg = "`typeOfProcessedData` overridden from parameter dictionary: value='";
                logMsg += enum2name_TypeOfProcessedData_or_throw(result);
                logMsg += "'";
                return logMsg;
            }());
        }
        else {

            // Deduce typeOfProcessedData from mars type
            if (marsType == "an") {
                result = tables::TypeOfProcessedData::AnalysisProducts;
            }
            else if (marsType == "fc") {
                result = tables::TypeOfProcessedData::ForecastProducts;
            }
            else if (marsType == "pf") {
                result = tables::TypeOfProcessedData::PerturbedForecastProducts;
            }
            else if (marsType == "cf") {
                result = tables::TypeOfProcessedData::ControlForecastProducts;
            }
            else if (marsType == "ssd" || marsType == "gsd") {
                result = tables::TypeOfProcessedData::ProcessedSatelliteObservations;
            }
            else {
                result = tables::TypeOfProcessedData::Missing;
            }

            // Emit OVERRIDE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`typeOfProcessedData` resolved from input dictionaries: value='";
                logMsg += enum2name_TypeOfProcessedData_or_throw(result);
                logMsg += "'";
                return logMsg;
            }());
        }

        // Success exit point
        return result;
    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(
            Mars2GribDeductionException("Failed to resolve `typeOfProcessedData` from input dictionaries", Here()));
    }
};


}  // namespace metkit::mars2grib::backend::deductions
