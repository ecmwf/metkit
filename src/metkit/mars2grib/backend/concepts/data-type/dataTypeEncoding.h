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
/// @file dataTypeOp.h
/// @brief Implementation of the GRIB `dataType` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **dataType concept** within the mars2grib backend.
///
/// The concept is responsible for populating GRIB keys in the
/// *Identification Section* related to the classification of the processed
/// data product, namely:
///
/// - `typeOfProcessedData`
/// - `productionStatusOfProcessedData`
///
/// The concept itself does not implement semantic deduction logic. Instead,
/// it delegates:
/// - semantic resolution to dedicated deduction functions
/// - value validation and encoding correctness to GRIB tables
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `data_typeApplicable`
/// - Runtime deduction via backend deductions
/// - Strict error handling with contextual concept exceptions
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of `concepts`
/// to avoid ambiguity and potential conflicts with the C++20 `concept` language
/// feature and related standard headers.
///
/// This is a deliberate design choice and must not be changed.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <string>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/data-type/dataTypeEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/productionStatusOfProcessedData.h"
#include "metkit/mars2grib/backend/deductions/typeOfProcessedData.h"

// Tables
#include "metkit/mars2grib/backend/tables/productionStatusOfProcessedData.h"
#include "metkit/mars2grib/backend/tables/typeOfProcessedData.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {


///
/// @brief Compile-time applicability predicate for the `dataType` concept.
///
/// This predicate determines whether the `dataType` concept is applicable
/// for a given combination of:
/// - encoding stage
/// - GRIB section
/// - concept variant
///
/// Applicability is evaluated entirely at compile time and is used by the
/// concept dispatcher to ensure that only valid concept instantiations occur.
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Data type concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
/// @note
/// The default applicability rule enables the concept only when:
/// - `Variant == DataTypeType::Default`
/// - `Stage == StagePreset`
/// - `Section == SecIdentificationSection`
///
/// Users may override or specialize this predicate to alter applicability.
///
template <std::size_t Stage, std::size_t Section, DataTypeType Variant>
constexpr bool dataTypeApplicable() {

    return ((Variant == DataTypeType::Default) && (Stage == StageOverride) && (Section == SecIdentificationSection));
}


///
/// @brief Execute the `dataType` concept operation.
///
/// This function implements the runtime logic of the GRIB `dataType` concept.
/// When applicable, it:
///
/// 1. Deduces the `typeOfProcessedData` from MARS and parameter dictionaries.
/// 2. Deduces the `productionStatusOfProcessedData` from MARS and parameter dictionaries.
/// 3. Encodes both values into the GRIB Identification Section.
///
/// The concept acts as a **pure orchestration layer**:
/// - All semantic logic is delegated to deduction functions.
/// - All value correctness is guaranteed by table-backed enumerations.
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Data type concept variant
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam ParDict_t  Type of the parameter dictionary
/// @tparam OptDict_t  Type of the options dictionary
/// @tparam OutDict_t  Type of the GRIB output dictionary
///
/// @param[in]  mars MARS input dictionary
/// @param[in]  par  Parameter dictionary
/// @param[in]  opt  Options dictionary
/// @param[out] out  Output GRIB dictionary to be populated
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
/// If:
/// - the concept is called when not applicable
/// - a deduction fails
/// - a GRIB key cannot be set
///
/// @note
/// - All runtime errors are wrapped with full concept context
/// (concept name, variant, stage, section).
/// - This concept does not perform validation beyond what is enforced
/// by the deduction and table layers.
///
/// @see dataTypeApplicable
///
template <std::size_t Stage, std::size_t Section, DataTypeType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void DataTypeOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (dataTypeApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(dataType);

            // Deductions
            tables::TypeOfProcessedData typeOfProcessedData =
                deductions::resolve_TypeOfProcessedData_or_throw(mars, par, opt);
            tables::ProductionStatusOfProcessedData productionStatusOfProcessedData =
                deductions::resolve_ProductionStatusOfProcessedData_or_throw(mars, par, opt);

            // Encoding
            // @todo -> set_or_throw<std::string>(out, "typeOfProcessedData",
            // enum2name_TypeOfProcessedData_or_throw(typeOfProcessedData));
            set_or_throw<long>(out, "typeOfProcessedData", static_cast<long>(typeOfProcessedData));
            set_or_throw<long>(out, "productionStatusOfProcessedData",
                               static_cast<long>(productionStatusOfProcessedData));
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(dataType, "Unable to set `dataType` concept...");
        }

        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(dataType, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
