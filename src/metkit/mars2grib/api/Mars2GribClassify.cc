/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/api/Mars2GribClassify.h"

#include <sstream>

#include "metkit/mars2grib/CoreOperations.h"
#include "metkit/mars2grib/api/Mars2GribApiErrorHandling.h"
#include "metkit/mars2grib/api/readOptionsFromInitializerList.h"
#include "metkit/mars2grib/api/readOptionsFromLocalConfiguration.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/frontend/resolution/resolveActiveConcepts.h"
#include "metkit/mars2grib/utils/TemporalArithmetic.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_options.h"

namespace metkit::mars2grib {

namespace exceptions = metkit::mars2grib::utils::exceptions;

namespace {

using StatisticalProcessing = backend::tables::TypeOfStatisticalProcessing;
using ProductTimeSpec = backend::models::product_time_spec::ProductTimeSpec;

const char* statisticalProcessingName(StatisticalProcessing value) {
    switch (value) {
        case StatisticalProcessing::Average:
            return "Average";
        case StatisticalProcessing::Accumulation:
            return "Accumulation";
        case StatisticalProcessing::Maximum:
            return "Maximum";
        case StatisticalProcessing::Minimum:
            return "Minimum";
        case StatisticalProcessing::DifferenceEndMinusStart:
            return "DifferenceEndMinusStart";
        case StatisticalProcessing::RootMeanSquare:
            return "RootMeanSquare";
        case StatisticalProcessing::StandardDeviation:
            return "StandardDeviation";
        case StatisticalProcessing::Covariance:
            return "Covariance";
        case StatisticalProcessing::DifferenceStartMinusEnd:
            return "DifferenceStartMinusEnd";
        case StatisticalProcessing::Ratio:
            return "Ratio";
        case StatisticalProcessing::StandardizedAnomaly:
            return "StandardizedAnomaly";
        case StatisticalProcessing::Summation:
            return "Summation";
        case StatisticalProcessing::ReturnPeriod:
            return "ReturnPeriod";
        case StatisticalProcessing::Median:
            return "Median";
        case StatisticalProcessing::Severity:
            return "Severity";
        case StatisticalProcessing::Mode:
            return "Mode";
        case StatisticalProcessing::IndexProcessing:
            return "IndexProcessing";
        case StatisticalProcessing::Missing:
            return "Instant";
    }

    mars2gribUnreachable();
}

std::string productTimeSpecClassification(const ProductTimeSpec& spec) {
    using backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using utils::time_arithmetic::convertToSeconds;

    if (spec.shapeType() == ProductTimeSpecShapeKind::Instant) {
        return "Instant()";
    }

    std::string classification;
    for (auto it = spec.windows().values.rbegin(); it != spec.windows().values.rend(); ++it) {
        std::ostringstream out;
        out << statisticalProcessingName(it->typeOfStatisticalProcessing) << '(' << convertToSeconds(it->timeRange);
        if (!classification.empty()) {
            out << ',' << classification;
        }
        out << ')';
        classification = out.str();
    }
    return classification;
}

ProductTimeSpecResult computeProductTimeSpecResult(const eckit::LocalConfiguration& inputMars,
                                                   const eckit::LocalConfiguration& inputMisc,
                                                   const Options& options, const eckit::Value& language) {
    eckit::LocalConfiguration scratchMars;
    eckit::LocalConfiguration scratchMisc;
    auto [mars, misc] = CoreOperations::normalize_if_enabled(inputMars, inputMisc, options, language, scratchMars,
                                                              scratchMisc);
    const auto activeConcepts = frontend::resolution::resolve_ActiveConcepts_or_throw(mars, options);
    const ProductTimeSpec spec{detail::innerStatisticalProcessing(activeConcepts), mars, misc, options};
    return ProductTimeSpecResult{spec.to_json(), productTimeSpecClassification(spec)};
}

}  // namespace

Mars2GribClassify::Mars2GribClassify() : opts_{} {}

Mars2GribClassify::Mars2GribClassify(const Options& opts) : opts_{opts} {}

Mars2GribClassify::Mars2GribClassify(const eckit::LocalConfiguration& opts) : opts_{detail::readOptions(opts)} {}

Mars2GribClassify::Mars2GribClassify(OptionList opts) : opts_{detail::readOptions(opts)} {}

std::string Mars2GribClassify::computeActiveConcepts(const eckit::LocalConfiguration& mars,
                                                     const eckit::LocalConfiguration& misc) {
    return exceptions::withMars2GribApiErrorHandling<std::string>(
        "Mars2GribClassify::computeActiveConcepts", opts_,
        [&]() { return CoreOperations::computeActiveConcepts(mars, misc, opts_, language_); }, Here());
}

std::string Mars2GribClassify::computeActiveConcepts(const eckit::LocalConfiguration& mars) {
    return computeActiveConcepts(mars, eckit::LocalConfiguration{});
}

ProductTimeSpecResult Mars2GribClassify::computeProductTimeSpec(const eckit::LocalConfiguration& mars,
                                                                const eckit::LocalConfiguration& misc) {
    return exceptions::withMars2GribApiErrorHandling<ProductTimeSpecResult>(
        "Mars2GribClassify::computeProductTimeSpec", opts_,
        [&]() { return computeProductTimeSpecResult(mars, misc, opts_, language_); }, Here());
}

ProductTimeSpecResult Mars2GribClassify::computeProductTimeSpec(const eckit::LocalConfiguration& mars) {
    return computeProductTimeSpec(mars, eckit::LocalConfiguration{});
}

}  // namespace metkit::mars2grib
