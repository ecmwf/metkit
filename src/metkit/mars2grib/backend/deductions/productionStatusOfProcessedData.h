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
/// @file productionStatusOfProcessedData.h
/// @brief Deduction of the GRIB `productionStatusOfProcessedData` key (Code Table 1.3).
///
/// This header defines the deduction used by the mars2grib backend to resolve
/// `productionStatusOfProcessedData`, which characterises the production status
/// of the processed data product.
///
/// The deduction currently implements a minimal mapping based on MARS metadata:
/// - `mars["class"] == "d1"`  → `DestinationEarth`
/// - `mars["class"] == "e6"`  → `ReanalysisProducts`
/// - otherwise               → `OperationalProducts` (default)
///
/// The resolved value is returned as a strongly typed table enum and is intended
/// to be consumed by concept operations (deductions do not encode GRIB keys).
///
/// Logging policy:
/// - RESOLVE: value derived via deduction logic from input dictionaries
/// - DEFAULT: value defaulted to a predefined constant due to missing input
///
/// Error handling:
/// - missing required inputs or unexpected failures throw `Mars2GribDeductionException`
/// - underlying errors are preserved via nested exception propagation
///
/// @section References
/// Concept:
/// - @ref dataTypeEncoding.h
///
/// Related deductions:
/// - @ref typeOfProcessedData.h
///
/// @ingroup mars2grib_backend_deductions
///
#pragma once

// System includes
#include <string>
#include <optional>

// Table includes
#include "metkit/mars2grib/backend/tables/productionStatusOfProcessedData.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Core deduction includes
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"


namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the GRIB `productionStatusOfProcessedData` key from input dictionaries.
///
/// This deduction resolves `productionStatusOfProcessedData` using MARS metadata.
///
/// Current rules:
/// - If `mars["class"] == "d1"`, return `DestinationEarth`.
/// - If `mars["class"] == "e6"`, return `ReanalysisProducts`.
/// - Otherwise, return `OperationalProducts`.
///
/// @section Deduction contract
/// - Reads: `mars["class"]`
/// - Writes: none
/// - Side effects: logging (RESOLVE,DEFAULT)
/// - Failure mode: throws
///
/// @tparam MarsDict_t
/// Type of the MARS dictionary; must provide key `class`.
///
/// @tparam ParDict_t
/// Type of the parameter dictionary (currently unused).
///
/// @tparam OptDict_t
/// Type of the options dictionary (currently unused).
///
/// @param[in] mars
/// MARS dictionary used to resolve `productionStatusOfProcessedData`.
///
/// @param[in] par
/// Parameter dictionary (unused).
///
/// @param[in] opt
/// Options dictionary (unused).
///
/// @return
/// The resolved `tables::ProductionStatusOfProcessedData` enumeration value.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
/// If required inputs are missing or if any unexpected error occurs during deduction.
///
/// @note
/// This deduction currently does not implement any override path via `par`.
/// If an override mechanism is introduced, successful override must emit an OVERRIDE log entry.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
tables::ProductionStatusOfProcessedData resolve_ProductionStatusOfProcessedData_or_throw(const MarsDict_t& mars,
                                                                                         const ParDict_t& par,
                                                                                         const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    try {

        // TODO: Here we set the default to operational product; will need to clarify exact logic with DGOV
        // It will probably need to be inferred from "type", "class", "stream"
        auto productionStatusOfProcessedDataDefault = tables::ProductionStatusOfProcessedData::OperationalProducts;
        std::optional<tables::ProductionStatusOfProcessedData> productionStatusOfProcessedData = std::nullopt;

        // Retrieve mandatory MARS class
        const auto marsClass = get_or_throw<std::string>(mars, "class");

        // Deduce productionStatusOfProcessedData from mars class
        if (marsClass == "d1") {
            // This is mandatory for DestinE because it is used inside eccodes to allocate the proper "localUseSection"
            // template. Setting this keyword reallocate the local use section.
            productionStatusOfProcessedData = tables::ProductionStatusOfProcessedData::DestinationEarth;
        }
        else if (marsClass == "e6") {
            // Special handling for ERA6
            productionStatusOfProcessedData = tables::ProductionStatusOfProcessedData::ReanalysisProducts;
        }

        // Emit log entry based on deduction outcome
        if (productionStatusOfProcessedData.has_value()) {
            // Emit RESOLVE log entry
            MARS2GRIB_LOG_RESOLVE([&]() {
                std::string logMsg = "`productionStatusOfProcessedData` resolved from input dictionaries: value='";
                logMsg += enum2name_ProductionStatusOfProcessedData_or_throw(productionStatusOfProcessedData.value());
                logMsg += "'";
                return logMsg;
            }());

            // Success exit point
            return productionStatusOfProcessedData.value();
        }
        else {
            // Default to operational product
            productionStatusOfProcessedData = productionStatusOfProcessedDataDefault;

            // Emit DEFAULT log entry
            MARS2GRIB_LOG_DEFAULT([&]() {
                std::string logMsg = "`productionStatusOfProcessedData` defaulted to operational product: value='";
                logMsg += enum2name_ProductionStatusOfProcessedData_or_throw(productionStatusOfProcessedData.value());
                logMsg += "'";
                return logMsg;
            }());

            // Success exit point
            return productionStatusOfProcessedData.value();


        }

    }
    catch (...) {

        // Rethrow nested exceptions
        std::throw_with_nested(Mars2GribDeductionException(
            "Failed to resolve `productionStatusOfProcessedData` from input dictionaries", Here()));
    }

    // Remove compiler warning
    mars2gribUnreachable();
};


}  // namespace metkit::mars2grib::backend::deductions
