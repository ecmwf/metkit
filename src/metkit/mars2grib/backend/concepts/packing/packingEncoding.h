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
/// @file packingOp.h
/// @brief Implementation of the GRIB `packing` concept operation.
///
/// This header defines the applicability rules and execution logic for the
/// **packing concept** within the mars2grib backend.
///
/// The packing concept is responsible for configuring the GRIB
/// *Data Representation Section* according to the selected packing algorithm.
/// It validates the underlying data representation template and sets the
/// required packing-specific GRIB keys.
///
/// Supported packing variants include:
/// - Simple packing
/// - CCSDS packing
/// - Spectral complex packing
///
/// The implementation follows the standard mars2grib concept model:
/// - Compile-time applicability via `packingApplicable`
/// - Variant-specific runtime validation
/// - Deduction of packing parameters from MARS and parameter dictionaries
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
#include "metkit/mars2grib/backend/concepts/packing/packingEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"

// Checks
#include "metkit/mars2grib/backend/checks/matchDataRepresentationTemplateNumber.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/bitsPerValue.h"
#include "metkit/mars2grib/backend/deductions/laplacianOperator.h"
#include "metkit/mars2grib/backend/deductions/subSetTrunc.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Compile-time applicability predicate for the `packing` concept.
///
/// The default applicability enables this concept only when:
/// - `Stage == StagePreset`
/// - `Section == SecDataRepresentationSection`
///
/// @tparam Stage   Encoding stage (compile-time constant)
/// @tparam Section GRIB section index (compile-time constant)
/// @tparam Variant Packing concept variant
///
/// @return `true` if the concept is applicable for the given parameters,
/// `false` otherwise.
///
template <std::size_t Stage, std::size_t Section, PackingType Variant>
constexpr bool packingApplicable() {
    return (Stage == StagePreset && Section == SecDataRepresentationSection);
}


///
/// @brief Execute the `packing` concept operation.
///
/// When applicable, this concept:
/// 1. Validates the GRIB data representation template number.
/// 2. Deduces packing-specific parameters from the input dictionaries.
/// 3. Encodes the corresponding GRIB keys into the output dictionary.
///
/// The exact behavior depends on the selected packing variant:
///
/// - **Simple packing**
/// - Template: 0
/// - Keys set: `bitsPerValue`
///
/// - **CCSDS packing**
/// - Template: 42
/// - Keys set: `bitsPerValue`
///
/// - **Spectral complex packing**
/// - Template: 51
/// - Keys set:
/// - `bitsPerValue`
/// - `laplacianOperator`
/// - `subSetJ`, `subSetK`, `subSetM`
/// - `TS`
///
/// If the concept is invoked when not applicable, a
/// `Mars2GribConceptException` is thrown.
///
/// @tparam Stage      Encoding stage (compile-time constant)
/// @tparam Section    GRIB section index (compile-time constant)
/// @tparam Variant    Packing concept variant
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
/// - the GRIB data representation template does not match the variant
/// - any deduction or encoding step fails
///
/// @note
/// This concept does not rely on any pre-existing GRIB header state.
///
/// @see packingApplicable
///
template <std::size_t Stage, std::size_t Section, PackingType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void PackingOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (packingApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(packing);

            // =============================================================
            // Variant-specific logic
            // =============================================================
            if constexpr (Variant == PackingType::Simple) {

                // Check sample structure
                validation::match_DataRepresentationTemplateNumber_or_throw(opt, out, {0});

                // Get bits per value
                long bitsPerValue = deductions::resolve_BitsPerValueGridded_or_throw(mars, par, opt);

                // Set bits per value
                set_or_throw<long>(out, "bitsPerValue", bitsPerValue);
            }

            if constexpr (Variant == PackingType::Ccsds) {

                // Check sample structure
                validation::match_DataRepresentationTemplateNumber_or_throw(opt, out, {42});

                // Get bits per value
                long bitsPerValue = deductions::resolve_BitsPerValueGridded_or_throw(mars, par, opt);

                // Set bits per value
                set_or_throw<long>(out, "bitsPerValue", bitsPerValue);
            }

            if constexpr (Variant == PackingType::SpectralComplex) {

                // Check sample structure
                validation::match_DataRepresentationTemplateNumber_or_throw(opt, out, {51});

                // Get bits per value
                long bitsPerValue        = deductions::resolve_BitsPerValueSpectral_or_throw(mars, par, opt);
                double laplacianOperator = deductions::resolve_LaplacianOperator_or_throw(mars, par, opt);
                long subSetTruncation    = deductions::resolve_SubSetTruncation_or_throw(mars, par, opt);

                // Set bits per value
                set_or_throw<long>(out, "bitsPerValue", bitsPerValue);
                set_or_throw<double>(out, "laplacianOperator", laplacianOperator);
                set_or_throw<long>(out, "subSetJ", subSetTruncation);
                set_or_throw<long>(out, "subSetK", subSetTruncation);
                set_or_throw<long>(out, "subSetM", subSetTruncation);
                set_or_throw<long>(out, "TS", (subSetTruncation + 1) * (subSetTruncation + 2));
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(packing, "Unable to set `packing` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(packing, "Concept called when not applicable...");

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
