/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file postProcessingEncoding.h
/// @brief `postProcessing` concept: the `postproc` block of PDT 4.89/4.90/4.133/4.135.
///
/// The post-processing keys are not part of the MARS request. For MARS type=pfc (ecPoint), the values of the
/// operational ECMWF GRIB2 products are used: typeOfPostProcessing 206 (weather-type-subgrid-calibration,
/// ecCodes postProcessingConcept.def), inputProcessIdentifier 16, inputOriginatingCentre 98.

#pragma once

#include <cstddef>
#include <string>

#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/post-processing/postProcessingEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <std::size_t Stage, std::size_t Section, PostProcessingType Variant>
constexpr bool postProcessingApplicable() {
    return (Stage == StagePreset) && (Section == SecProductDefinitionSection);
}

template <std::size_t Stage, std::size_t Section, PostProcessingType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void PostProcessingOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
    (void)par;
    (void)opt;

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if constexpr (postProcessingApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(postProcessing);

            const auto marsType = get_or_throw<std::string>(mars, "type");
            if (marsType != "pfc") {
                throw Mars2GribGenericException("No post-processing keys defined for MARS type `" + marsType + "`",
                                                Here());
            }

            set_or_throw<long>(out, "inputProcessIdentifier", 16);
            set_or_throw<long>(out, "inputOriginatingCentre", 98);
            set_or_throw<long>(out, "typeOfPostProcessing", 206);
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(postProcessing, "Unable to set `postProcessing` concept...");
        }

        return;
    }

    MARS2GRIB_CONCEPT_THROW(postProcessing, "Concept called when not applicable...");
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
