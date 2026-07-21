/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file ProductTimeSpecModelJsonUtils.h
/// @brief Internal JSON helpers for final ProductTimeSpec diagnostics.
///

#pragma once

#include <sstream>
#include <string>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecTimeIncrementClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecWindows.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"

namespace metkit::mars2grib::backend::models::detail {

inline std::string productTimeSpecAnchorTypeName(TimeAnchorKind value) {
    switch (value) {
        case TimeAnchorKind::LabelOnly:
            return "LabelOnly";
        case TimeAnchorKind::Hindcast:
            return "Hindcast";
        case TimeAnchorKind::ForecastAnchor:
            return "ForecastAnchor";
        case TimeAnchorKind::HindcastForecastAnchor:
            return "HindcastForecastAnchor";
    }

    return "InvalidTimeAnchorKind";
}

inline std::string productTimeSpecShapeTypeName(ProductTimeSpecShapeKind value) {
    switch (value) {
        case ProductTimeSpecShapeKind::Instant:
            return "Instant";
        case ProductTimeSpecShapeKind::StandardSingleLoop:
            return "StandardSingleLoop";
        case ProductTimeSpecShapeKind::MultiLoop:
            return "MultiLoop";
        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
            return "FakeDoubleLoopSingleLoop";
        case ProductTimeSpecShapeKind::FromStartSingleLoop:
            return "FromStartSingleLoop";
        case ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop:
            return "FakeSingleLoopDoubleLoop";
    }

    return "InvalidProductTimeSpecShapeKind";
}

inline std::string productTimeSpecIncrementTypeName(TimeIncrementKind value) {
    switch (value) {
        case TimeIncrementKind::NoIncrement:
            return "NoIncrement";
        case TimeIncrementKind::ExplicitIncrement:
            return "ExplicitIncrement";
        case TimeIncrementKind::DefaultedIncrement:
            return "DefaultedIncrement";
        case TimeIncrementKind::AifsPureMissingIncrement:
            return "AifsPureMissingIncrement";
    }

    return "InvalidTimeIncrementKind";
}

inline std::string productTimeSpecDateTimeJson(const eckit::DateTime& value) {
    return jsonQuote_modelInput(value.iso(true));
}

inline std::string productTimeSpecAnchorJson(const ProductTimeSpecAnchor& value) {
    std::ostringstream out;
    out << '{'
        << jsonQuote_modelInput("labelDateTime") << ':' << productTimeSpecDateTimeJson(value.labelDateTime) << ','
        << jsonQuote_modelInput("initialConditionsDateTime") << ':'
        << productTimeSpecDateTimeJson(value.initialConditionsDateTime) << ','
        << jsonQuote_modelInput("referenceDateTime") << ':'
        << productTimeSpecDateTimeJson(value.referenceDateTime) << ','
        << jsonQuote_modelInput("anchorType") << ':'
        << jsonQuote_modelInput(productTimeSpecAnchorTypeName(value.anchorType))
        << '}';
    return out.str();
}

inline std::string productTimeSpecDomainJson(const ProductTimeSpecDomain& value) {
    std::ostringstream out;
    out << '{'
        << jsonQuote_modelInput("domainStartDateTime") << ':'
        << productTimeSpecDateTimeJson(value.domainStartDateTime) << ','
        << jsonQuote_modelInput("domainEndDateTime") << ':'
        << productTimeSpecDateTimeJson(value.domainEndDateTime)
        << '}';
    return out.str();
}

inline std::string productTimeSpecWindowJson(const ProductTimeSpecWindow& value) {
    std::ostringstream out;
    out << '{'
        << jsonQuote_modelInput("typeOfStatisticalProcessing") << ':'
        << jsonQuote_modelInput(
               tables::enum2name_TypeOfStatisticalProcessing_or_throw(value.typeOfStatisticalProcessing)) << ','
        << jsonQuote_modelInput("timeRange") << ':'
        << durationJson_modelInput(value.timeRange) << ','
        << jsonQuote_modelInput("timeIncrement") << ':'
        << durationJson_modelInput(value.timeIncrement)
        << '}';
    return out.str();
}

inline std::string productTimeSpecWindowsJson(const ProductTimeSpecWindows& value) {
    std::ostringstream out;
    out << '[';
    for (std::size_t i = 0; i < value.values.size(); ++i) {
        if (i != 0) {
            out << ',';
        }
        out << productTimeSpecWindowJson(value.values[i]);
    }
    out << ']';
    return out.str();
}

}  // namespace metkit::mars2grib::backend::models::detail
