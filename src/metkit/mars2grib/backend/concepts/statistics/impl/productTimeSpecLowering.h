/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
#pragma once

#include <vector>

#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_::impl {

struct ProductTimeSpecStatisticalProcessing {
    long numberOfTimeRanges{0};
    std::vector<long> typeOfStatisticalProcessing;
    std::vector<long> typeOfTimeIncrement;
    std::vector<long> indicatorOfUnitForTimeRange;
    std::vector<long> lengthOfTimeRange;
    std::vector<long> indicatorOfUnitForTimeIncrement;
    std::vector<long> timeIncrement;
};

inline ProductTimeSpecStatisticalProcessing lower_ProductTimeSpecStatistics_or_throw(
    const product_time_spec::ProductTimeSpec& spec) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (spec.kind() == product_time_spec::ProductTimeSpecKind::Instant) {
        throw Mars2GribDeductionException(
            "Statistics backend cannot lower an Instant ProductTimeSpec", Here());
    }

    ProductTimeSpecStatisticalProcessing out;
    out.numberOfTimeRanges = static_cast<long>(spec.numberOfTimeRanges());
    out.typeOfStatisticalProcessing.reserve(spec.size());
    out.typeOfTimeIncrement.reserve(spec.size());
    out.indicatorOfUnitForTimeRange.reserve(spec.size());
    out.lengthOfTimeRange.reserve(spec.size());
    out.indicatorOfUnitForTimeIncrement.reserve(spec.size());
    out.timeIncrement.reserve(spec.size());

    for (const auto& window : spec) {
        out.typeOfStatisticalProcessing.push_back(
            static_cast<long>(window.typeOfStatisticalProcessing));
        out.typeOfTimeIncrement.push_back(
            static_cast<long>(window.typeOfTimeIncrement));
        out.indicatorOfUnitForTimeRange.push_back(
            static_cast<long>(window.timeRange.unit));
        out.lengthOfTimeRange.push_back(window.timeRange.length);
        out.indicatorOfUnitForTimeIncrement.push_back(
            static_cast<long>(window.timeIncrement.unit));
        out.timeIncrement.push_back(window.timeIncrement.length);
    }

    return out;
}

}  // namespace metkit::mars2grib::backend::concepts_::impl
