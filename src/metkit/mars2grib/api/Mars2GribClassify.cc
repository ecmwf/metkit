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
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib {

namespace exceptions = metkit::mars2grib::utils::exceptions;

namespace {

using StatisticalProcessing = backend::tables::TypeOfStatisticalProcessing;
using ProductTimeSpec       = backend::models::product_time_spec::ProductTimeSpec;

const char* statisticalProcessingName(StatisticalProcessing value, utils::profiling::NoProfileContext& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const char* result = nullptr;
    switch (value) {
        case StatisticalProcessing::Average:
            result = "Average";
            break;
        case StatisticalProcessing::Accumulation:
            result = "Accumulation";
            break;
        case StatisticalProcessing::Maximum:
            result = "Maximum";
            break;
        case StatisticalProcessing::Minimum:
            result = "Minimum";
            break;
        case StatisticalProcessing::DifferenceEndMinusStart:
            result = "DifferenceEndMinusStart";
            break;
        case StatisticalProcessing::RootMeanSquare:
            result = "RootMeanSquare";
            break;
        case StatisticalProcessing::StandardDeviation:
            result = "StandardDeviation";
            break;
        case StatisticalProcessing::Covariance:
            result = "Covariance";
            break;
        case StatisticalProcessing::DifferenceStartMinusEnd:
            result = "DifferenceStartMinusEnd";
            break;
        case StatisticalProcessing::Ratio:
            result = "Ratio";
            break;
        case StatisticalProcessing::StandardizedAnomaly:
            result = "StandardizedAnomaly";
            break;
        case StatisticalProcessing::Summation:
            result = "Summation";
            break;
        case StatisticalProcessing::ReturnPeriod:
            result = "ReturnPeriod";
            break;
        case StatisticalProcessing::Median:
            result = "Median";
            break;
        case StatisticalProcessing::Severity:
            result = "Severity";
            break;
        case StatisticalProcessing::Mode:
            result = "Mode";
            break;
        case StatisticalProcessing::IndexProcessing:
            result = "IndexProcessing";
            break;
        case StatisticalProcessing::Missing:
            result = "Instant";
            break;
    }

    if (result == nullptr) {
        mars2gribUnreachable();
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

std::string productTimeSpecClassification(const ProductTimeSpec& spec, utils::profiling::NoProfileContext& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    using backend::models::product_time_spec::shape::ProductTimeSpecShapeKind;
    using utils::time_arithmetic::convertToSeconds;

    if (spec.shapeType(utils::profiling::callSite(cntx, Here())) == ProductTimeSpecShapeKind::Instant) {
        std::string result{"Instant()"};
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }

    std::string classification;
    for (auto it = spec.windows(utils::profiling::callSite(cntx, Here())).values.rbegin();
         it != spec.windows(utils::profiling::callSite(cntx, Here())).values.rend(); ++it) {
        std::ostringstream out;
        out << statisticalProcessingName(it->typeOfStatisticalProcessing, utils::profiling::callSite(cntx, Here()))
            << '(' << convertToSeconds(it->timeRange, utils::profiling::callSite(cntx, Here()));
        if (!classification.empty()) {
            out << ',' << classification;
        }
        out << ')';
        classification = out.str();
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return classification;
}

ProductTimeSpecResult computeProductTimeSpecResult(const eckit::LocalConfiguration& inputMars,
                                                   const eckit::LocalConfiguration& inputMisc, const Options& options,
                                                   const eckit::Value& language,
                                                   utils::profiling::NoProfileContext& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    eckit::LocalConfiguration scratchMars;
    eckit::LocalConfiguration scratchMisc;
    auto [mars, misc] =
        CoreOperations::normalize_if_enabled(inputMars, inputMisc, options, language, scratchMars, scratchMisc,
                                             utils::profiling::callSite(cntx, Here()));
    const auto activeConcepts = frontend::resolution::resolve_ActiveConcepts_or_throw(
        mars, options, utils::profiling::callSite(cntx, Here()));
    const ProductTimeSpec spec{
        detail::innerStatisticalProcessing(activeConcepts, utils::profiling::callSite(cntx, Here())), mars, misc,
        options, utils::profiling::callSite(cntx, Here())};
    ProductTimeSpecResult result{spec.to_json(utils::profiling::callSite(cntx, Here())),
                                 productTimeSpecClassification(spec, utils::profiling::callSite(cntx, Here()))};
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

long computeProductTimeSpecOuterTimeRangeInHours(const eckit::LocalConfiguration& inputMars,
                                                  const eckit::LocalConfiguration& inputMisc, const Options& options,
                                                   const eckit::Value& language,
                                                   utils::profiling::NoProfileContext& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    using backend::tables::TimeUnit;

    eckit::LocalConfiguration scratchMars;
    eckit::LocalConfiguration scratchMisc;
    auto [mars, misc] =
        CoreOperations::normalize_if_enabled(inputMars, inputMisc, options, language, scratchMars, scratchMisc,
                                             utils::profiling::callSite(cntx, Here()));
    const auto activeConcepts = frontend::resolution::resolve_ActiveConcepts_or_throw(
        mars, options, utils::profiling::callSite(cntx, Here()));
    const ProductTimeSpec spec{
        detail::innerStatisticalProcessing(activeConcepts, utils::profiling::callSite(cntx, Here())), mars, misc,
        options, utils::profiling::callSite(cntx, Here())};
    const auto& windows = spec.windows(utils::profiling::callSite(cntx, Here())).values;
    if (windows.empty() || windows.front().timeRange.unit != TimeUnit::Hour) {
        throw exceptions::Mars2GribGenericException("ProductTimeSpec outer time range is not available in hours",
                                                     Here());
    }
    const long result = windows.front().timeRange.length;
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace

Mars2GribClassify::Mars2GribClassify() : opts_{} {}

Mars2GribClassify::Mars2GribClassify(const Options& opts) : opts_{opts} {}

Mars2GribClassify::Mars2GribClassify(const eckit::LocalConfiguration& opts) : opts_{detail::readOptions(opts)} {}

Mars2GribClassify::Mars2GribClassify(OptionList opts) : opts_{detail::readOptions(opts)} {}

std::string Mars2GribClassify::computeActiveConcepts(const eckit::LocalConfiguration& mars,
                                                      const eckit::LocalConfiguration& misc) {
    utils::profiling::NoProfileContext cntx;
    return exceptions::withMars2GribApiErrorHandling<std::string>(
        "Mars2GribClassify::computeActiveConcepts", opts_,
        [&]() {
            return CoreOperations::computeActiveConcepts(mars, misc, opts_, language_,
                                                         utils::profiling::callSite(cntx, Here()));
        },
        Here());
}

std::string Mars2GribClassify::computeActiveConcepts(const eckit::LocalConfiguration& mars) {
    return computeActiveConcepts(mars, eckit::LocalConfiguration{});
}

ProductTimeSpecResult Mars2GribClassify::computeProductTimeSpec(const eckit::LocalConfiguration& mars,
                                                                 const eckit::LocalConfiguration& misc) {
    utils::profiling::NoProfileContext cntx;
    return exceptions::withMars2GribApiErrorHandling<ProductTimeSpecResult>(
        "Mars2GribClassify::computeProductTimeSpec", opts_,
        [&]() {
            return computeProductTimeSpecResult(mars, misc, opts_, language_,
                                                utils::profiling::callSite(cntx, Here()));
        },
        Here());
}

ProductTimeSpecResult Mars2GribClassify::computeProductTimeSpec(const eckit::LocalConfiguration& mars) {
    return computeProductTimeSpec(mars, eckit::LocalConfiguration{});
}

long Mars2GribClassify::computeOuterTimeRangeInHours(const eckit::LocalConfiguration& mars,
                                                      const eckit::LocalConfiguration& misc) {
    utils::profiling::NoProfileContext cntx;
    return exceptions::withMars2GribApiErrorHandling<long>(
        "Mars2GribClassify::computeOuterTimeRangeInHours", opts_,
        [&]() {
            return computeProductTimeSpecOuterTimeRangeInHours(mars, misc, opts_, language_,
                                                               utils::profiling::callSite(cntx, Here()));
        },
        Here());
}

long Mars2GribClassify::computeOuterTimeRangeInHours(const eckit::LocalConfiguration& mars) {
    return computeOuterTimeRangeInHours(mars, eckit::LocalConfiguration{});
}

}  // namespace metkit::mars2grib
