/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>

#include "metkit/mars2grib/backend/concepts/quantile/quantileEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

template <std::size_t Stage, std::size_t Section, QuantileType Variant>
constexpr bool quantileApplicable() {
    return false;
}

/// Placeholder operation. No callback is registered while applicability is false.
///
/// TODO: deduction for the MARS `quantile` keyword, `<quantileValue>:<totalNumberOfQuantiles>` (e.g. `1:3`):
///       totalNumberOfQuantiles = 3, quantileValue = 1
template <std::size_t Stage, std::size_t Section, QuantileType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void QuantileOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {
    (void)mars;
    (void)par;
    (void)opt;
    (void)out;

    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    MARS2GRIB_CONCEPT_THROW(quantile, "Concept called when not applicable...");
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
