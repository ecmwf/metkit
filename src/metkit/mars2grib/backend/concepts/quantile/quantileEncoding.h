/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file quantileEncoding.h
/// @brief `quantile` concept: `totalNumberOfQuantiles` and `quantileValue` of PDT 4.86/4.87/4.89/4.90/4.132-4.135.
///
/// Both keys come from the MARS `quantile` keyword, `<quantileValue>:<totalNumberOfQuantiles>` (e.g. `1:3`).

#pragma once

#include <cstddef>
#include <string>

#include "metkit/mars/Quantile.h"
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/quantile/quantileEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <std::size_t Stage, std::size_t Section, QuantileType Variant>
constexpr bool quantileApplicable() {
    return (Stage == StagePreset) && (Section == SecProductDefinitionSection);
}

template <std::size_t Stage, std::size_t Section, QuantileType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void QuantileOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
    (void)par;
    (void)opt;

    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (quantileApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(quantile);

            // `<quantileValue>:<totalNumberOfQuantiles>`, validated by metkit::Quantile
            const metkit::Quantile quantile{get_or_throw<std::string>(mars, "quantile")};

            set_or_throw<long>(out, "totalNumberOfQuantiles", quantile.den());
            set_or_throw<long>(out, "quantileValue", quantile.num());
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(quantile, "Unable to set `quantile` concept...");
        }

        return;
    }

    MARS2GRIB_CONCEPT_THROW(quantile, "Concept called when not applicable...");
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
