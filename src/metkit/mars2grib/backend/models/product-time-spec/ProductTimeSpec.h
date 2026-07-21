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
/// @file ProductTimeSpec.h
/// @brief Immutable backend-model `ProductTimeSpec` assembled from classified artifacts.
///
/// Exposes `ProductTimeSpec`, the backend-model type whose public templated
/// constructor starts from input dictionaries, assembles a normalized
/// `ProductTimeSpecInput`, classifies the temporal semantics, builds the staged
/// artifacts, validates final whole-object consistency, and then stores the
/// final immutable model state.
///
/// This header owns:
/// - the immutable backend-model `ProductTimeSpec` class;
/// - the public dictionary-taking constructor entry point;
/// - the private staged-construction flow from normalized input to final stored
///   components.
///
/// This header does NOT:
/// - implement the long classification, build, arithmetic, or consistency
///   helper logic inline; those responsibilities are delegated to the dedicated
///   public/detail stage headers.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <string>
#include <utility>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchorClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecConsistency.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecTimeIncrementClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecWindows.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecModelJsonUtils.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {

class ProductTimeSpec {
public:
    ///
    /// @brief Build one backend-model `ProductTimeSpec` from input dictionaries.
    ///
    /// @section Model-build contract
    ///   - Reads (MARS): all ProductTimeSpec-related source keys via
    ///                   `make_ProductTimeSpecInput_or_throw`
    ///   - Reads (par):  all ProductTimeSpec-related parameter keys via
    ///                   `make_ProductTimeSpecInput_or_throw`
    ///   - Reads (opt):  all ProductTimeSpec model-policy booleans and any
    ///                   deduction-owned options through the normalized input
    ///                   builder
    ///   - Writes:       none
    ///   - Side effects: deduction-layer logging only
    ///   - Failure mode: throws `Mars2GribModelException` (nested-with)
    ///
    /// This public constructor is the backend-model entry point. It first
    /// assembles a normalized `ProductTimeSpecInput` snapshot from the deduction
    /// layer and then delegates all classification, build, validation, and final
    /// member initialization to private construction stages.
    ///
    /// @tparam MarsDict_t   MARS dictionary type.
    /// @tparam ParDict_t    Parameter dictionary type.
    /// @tparam OptDict_t    Options dictionary type.
    ///
    /// @param[in] innerMostTypeOfStatisticalProcessing Caller-supplied
    ///            innermost statistical processing type.
    /// @param[in] mars  MARS dictionary.
    /// @param[in] par   Parameter dictionary.
    /// @param[in] opt   Options dictionary.
    ///
    /// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
    ///         any deduction or normalized-input assembly failure, with the
    ///         original cause attached via `std::throw_with_nested`.
    ///
    template <class MarsDict_t, class ParDict_t, class OptDict_t>
    ProductTimeSpec(tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing,
                    const MarsDict_t& mars,
                    const ParDict_t& par,
                    const OptDict_t& opt) :
        ProductTimeSpec(make_ProductTimeSpecInput_or_throw(
            innerMostTypeOfStatisticalProcessing,
            mars,
            par,
            opt)) {}

    /// @brief Return the resolved anchor classification.
    TimeAnchorKind anchorType() const noexcept { return anchorType_; }

    /// @brief Return the resolved shape classification.
    ProductTimeSpecShapeKind shapeType() const noexcept { return shapeType_; }

    /// @brief Return the resolved time-increment classification.
    TimeIncrementKind incrementType() const noexcept { return incrementType_; }

    /// @brief Return the resolved anchor artifact.
    const ProductTimeSpecAnchor& anchor() const noexcept { return anchor_; }

    /// @brief Return the resolved absolute support domain.
    const ProductTimeSpecDomain& domain() const noexcept { return domain_; }

    /// @brief Return the resolved canonical window sequence.
    const ProductTimeSpecWindows& windows() const noexcept { return windows_; }

    ///
    /// @brief Serialize the final immutable ProductTimeSpec as diagnostic JSON.
    ///
    /// This function is best-effort and never throws. It is intended for upper-
    /// layer diagnostic context so concept-level exceptions can attach the final
    /// resolved ProductTimeSpec state without risking a secondary exception.
    ///
    /// @return One JSON object string on success, or a stable fallback JSON
    ///         error object if serialization itself fails.
    ///
    std::string to_json() const noexcept {
        try {
            std::ostringstream out;
            out << '{'
                << detail::jsonQuote_modelInput("anchorType") << ':'
                << detail::jsonQuote_modelInput(detail::productTimeSpecAnchorTypeName(anchorType_)) << ','
                << detail::jsonQuote_modelInput("shapeType") << ':'
                << detail::jsonQuote_modelInput(detail::productTimeSpecShapeTypeName(shapeType_)) << ','
                << detail::jsonQuote_modelInput("incrementType") << ':'
                << detail::jsonQuote_modelInput(detail::productTimeSpecIncrementTypeName(incrementType_)) << ','
                << detail::jsonQuote_modelInput("anchor") << ':'
                << detail::productTimeSpecAnchorJson(anchor_) << ','
                << detail::jsonQuote_modelInput("domain") << ':'
                << detail::productTimeSpecDomainJson(domain_) << ','
                << detail::jsonQuote_modelInput("windows") << ':'
                << detail::productTimeSpecWindowsJson(windows_)
                << '}';
            return out.str();
        } catch (...) {
            return std::string{"{\"error\":\"ProductTimeSpec::to_json() failed while building diagnostic context\"}"};
        }
    }

private:
    struct ProductTimeSpecComponents {
        TimeAnchorKind anchorType{TimeAnchorKind::LabelOnly};
        ProductTimeSpecShapeKind shapeType{ProductTimeSpecShapeKind::Instant};
        TimeIncrementKind incrementType{TimeIncrementKind::NoIncrement};
        ProductTimeSpecAnchor anchor{};
        ProductTimeSpecDomain domain{};
        ProductTimeSpecWindows windows{};
    };

    ///
    /// @brief Build the staged initialization components from normalized input.
    ///
    /// This helper executes the backend-model ProductTimeSpec pipeline after a
    /// normalized input snapshot already exists:
    /// 1. classify anchor;
    /// 2. build anchor;
    /// 3. classify shape;
    /// 4. classify time increment;
    /// 5. build windows;
    /// 6. build domain;
    /// 7. validate final whole-object consistency;
    /// 8. return the immutable member bundle used by the final constructor.
    ///
    /// @param[in] input Complete normalized ProductTimeSpec input snapshot.
    /// @return Complete staged component bundle for final member initialization.
    ///
    /// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
    ///         any classification, build, or consistency failure, with the
    ///         original cause attached via `std::throw_with_nested`.
    ///
    static ProductTimeSpecComponents build_ProductTimeSpecComponents_or_throw(ProductTimeSpecInput input) {
        using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

        try {
            const TimeAnchorKind anchorType = classify_ProductTimeSpecAnchor_or_throw(input);
            ProductTimeSpecAnchor anchor = build_ProductTimeSpecAnchor_or_throw(input, anchorType);
            const ProductTimeSpecShapeKind shapeType = classify_ProductTimeSpecShape_or_throw(input);
            const TimeIncrementKind incrementType =
                classify_ProductTimeSpecTimeIncrement_or_throw(input, shapeType);
            ProductTimeSpecWindows windows =
                build_ProductTimeSpecWindows_or_throw(input, anchorType, shapeType, incrementType, anchor);
            ProductTimeSpecDomain domain =
                build_ProductTimeSpecDomain_or_throw(input, anchorType, shapeType, incrementType, anchor);

            validate_ProductTimeSpecConsistency_or_throw(
                input,
                anchorType,
                shapeType,
                incrementType,
                anchor,
                domain,
                windows);

            ProductTimeSpecComponents result;
            result.anchorType = anchorType;
            result.shapeType = shapeType;
            result.incrementType = incrementType;
            result.anchor = std::move(anchor);
            result.domain = std::move(domain);
            result.windows = std::move(windows);
            return result;
        } catch (...) {
            std::throw_with_nested(Mars2GribModelException(
                "Failed to build `ProductTimeSpec` staged components from normalized input",
                input.to_json(),
                Here()));
        }

        mars2gribUnreachable();
    }

    ///
    /// @brief Materialize the model from an already-built normalized input.
    ///
    /// This private constructor isolates the transition between the normalized
    /// input snapshot and the later staged component construction. It performs
    /// no direct dictionary access.
    ///
    /// @param[in] input Complete normalized ProductTimeSpec input snapshot.
    ///
    explicit ProductTimeSpec(ProductTimeSpecInput input) :
        ProductTimeSpec(build_ProductTimeSpecComponents_or_throw(std::move(input))) {}

    ///
    /// @brief Final private construction stage from staged components.
    ///
    /// This constructor is the single point at which immutable members are
    /// initialized from the fully resolved staged component bundle.
    ///
    /// @param[in] components Staged component bundle used for final member
    ///            initialization.
    ///
    explicit ProductTimeSpec(ProductTimeSpecComponents components) :
        anchorType_(components.anchorType),
        shapeType_(components.shapeType),
        incrementType_(components.incrementType),
        anchor_(std::move(components.anchor)),
        domain_(std::move(components.domain)),
        windows_(std::move(components.windows)) {}

    const TimeAnchorKind anchorType_;
    const ProductTimeSpecShapeKind shapeType_;
    const TimeIncrementKind incrementType_;
    const ProductTimeSpecAnchor anchor_;
    const ProductTimeSpecDomain domain_;
    const ProductTimeSpecWindows windows_;
};

}  // namespace metkit::mars2grib::backend::models
