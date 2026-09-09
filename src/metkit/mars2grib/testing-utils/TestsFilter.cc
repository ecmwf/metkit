/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/TestsFilter.h"

#include <algorithm>
#include <array>

#include "metkit/mars2grib/CoreOperations.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/frontend/resolution/resolveActiveConcepts.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"

namespace metkit::mars2grib::testing_utils {
namespace {

bool option(const eckit::LocalConfiguration& options, const std::string& name) {
    return options.has(name) ? options.getBool(name) : true;
}

}  // namespace

TestsFilter::TestsFilter(const eckit::LocalConfiguration& options) :
    filterPerturbedForecast_{option(options, "filter-perturbed-forecast")},
    filterModelLevel_{option(options, "filter-model-level")},
    filterFrequencyDirection_{option(options, "filter-frequency-direction")} {}

bool TestsFilter::filter(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                         const eckit::LocalConfiguration& opt) const {
    const auto activeConcepts = frontend::resolution::resolve_ActiveConcepts_or_throw(mars, opt);
    const backend::models::product_time_spec::ProductTimeSpec productTimeSpec{
        detail::innerStatisticalProcessing(activeConcepts), mars, misc, opt};
    (void)productTimeSpec;

    const std::array conditions{
        !filterPerturbedForecast_ || !mars.has("type") || mars.getString("type") != "pf" ||
            (mars.has("number") && mars.getLong("number") == 1),
        !filterModelLevel_ || !mars.has("levtype") || mars.getString("levtype") != "ml" || !mars.has("levelist") ||
            mars.getLong("levelist") == 1,
        !filterFrequencyDirection_ || !mars.has("frequency") || !mars.has("direction") ||
            (mars.getLong("frequency") == 1 && mars.getLong("direction") == 1),
    };

    return std::all_of(conditions.begin(), conditions.end(), [](bool condition) { return condition; });
}

}  // namespace metkit::mars2grib::testing_utils
