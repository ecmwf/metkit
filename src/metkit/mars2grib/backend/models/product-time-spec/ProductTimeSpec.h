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
/// Exposes:
/// - `ProductTimeAnchorSpec`, a reduced backend-model type that resolves only
///   the temporal anchor state from input dictionaries;
/// - `ProductTimeSpec`, the full backend-model type whose public templated
///   constructor starts from input dictionaries, assembles a normalized
///   `ProductTimeSpecInput`, classifies the temporal semantics, builds the
///   staged artifacts, and then stores the final immutable model state.
///
/// This header owns:
/// - the immutable backend-model `ProductTimeAnchorSpec` class;
/// - the immutable backend-model `ProductTimeSpec` class;
/// - the public dictionary-taking constructor entry point;
/// - the private staged-construction flow from normalized input to final stored
///   components.
///
/// This header does NOT:
/// - implement the long classification, build, or arithmetic
///   helper logic inline; those responsibilities are delegated to the dedicated
///   public/detail stage headers.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

#include <string>
#include <utility>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"


#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorRegistry.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainRegistry.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeRegistry.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::product_time_spec {

class ProductTimeAnchorSpec {
public:

    ///
    /// @brief Build one backend-model `ProductTimeAnchorSpec` from input
    ///        dictionaries.
    ///
    /// @section Model-build contract
    ///   - Reads (MARS): direct anchor source keys through anchor-only
    ///                   deductions
    ///   - Reads (par):  direct anchor source keys through anchor-only
    ///                   deductions
    ///   - Reads (opt):  only through deductions that require options
    ///   - Writes:       none
    ///   - Side effects: deduction-layer logging only
    ///   - Failure mode: throws `Mars2GribModelException` (nested-with)
    ///
    /// This reduced public constructor is the anchor-only backend-model entry
    /// point. It assembles a minimal normalized input snapshot containing only
    /// the direct anchor sources and then delegates classification and anchor
    /// materialization to the existing ProductTimeSpec anchor pipeline.
    ///
    /// @tparam MarsDict_t MARS dictionary type.
    /// @tparam ParDict_t  Parameter dictionary type.
    /// @tparam OptDict_t  Options dictionary type.
    /// @param[in] mars MARS dictionary.
    /// @param[in] par  Parameter dictionary.
    /// @param[in] opt  Options dictionary.
    /// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
    ///         any deduction or anchor-build failure, with the original cause
    ///         attached via `std::throw_with_nested`.
    ///
    template <class MarsDict_t, class ParDict_t, class OptDict_t>
    ProductTimeAnchorSpec(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt) :
        ProductTimeAnchorSpec(make_ProductTimeAnchorSpecInput_or_throw(mars, par, opt)) {}

    /// @brief Return the resolved anchor classification.
    anchor::ProductTimeSpecAnchorKind anchorType() const noexcept { return anchorType_; }

    /// @brief Return the resolved anchor artifact.
    const anchor::ProductTimeSpecAnchor& anchor() const noexcept { return anchor_; }

    ///
    /// @brief Serialize the final immutable ProductTimeAnchorSpec as diagnostic
    ///        JSON.
    ///
    /// This function is best-effort and never throws. It is intended for upper-
    /// layer diagnostic context so concept-level exceptions can attach the final
    /// resolved anchor state without risking a secondary exception.
    ///
    /// @return One JSON object string on success, or a stable fallback JSON
    ///         error object if serialization itself fails.
    ///
    std::string to_json() const noexcept {
        try {
            std::ostringstream out;
            out << '{' << detail::jsonQuote_modelInput("anchorType") << ':'
                << detail::jsonQuote_modelInput(anchor::productTimeSpecAnchorTypeName(anchorType_)) << ','
                << detail::jsonQuote_modelInput("anchor") << ':' << anchor::productTimeSpecAnchorJson(anchor_) << '}';
            return out.str();
        }
        catch (...) {
            return std::string{
                "{\"error\":\"ProductTimeAnchorSpec::to_json() failed while building diagnostic context\"}"};
        }
    }

private:

    struct ProductTimeAnchorSpecComponents {
        anchor::ProductTimeSpecAnchorKind anchorType{anchor::ProductTimeSpecAnchorKind::ForecastAnalysis};
        anchor::ProductTimeSpecAnchor anchor{};
    };

    ///
    /// @brief Build one reduced normalized `ProductTimeSpecInput` for anchor resolution.
    ///
    /// This helper resolves only the direct anchor-source fields required by the
    /// shared ProductTimeSpec anchor classifier and builder. All non-anchor
    /// fields remain default-initialized, except
    /// `innerMostTypeOfStatisticalProcessing`, which is explicitly set to
    /// `Missing` so the shared input object remains fully initialized.
    ///
    /// @tparam MarsDict_t MARS dictionary type.
    /// @tparam ParDict_t  Parameter dictionary type.
    /// @tparam OptDict_t  Options dictionary type.
    /// @param[in] mars MARS dictionary.
    /// @param[in] par  Parameter dictionary.
    /// @param[in] opt  Options dictionary.
    /// @return Complete normalized anchor-only `ProductTimeSpecInput` snapshot.
    /// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
    ///         any deduction failure, with the original cause attached via
    ///         `std::throw_with_nested`.
    ///
    template <class MarsDict_t, class ParDict_t, class OptDict_t>
    static ProductTimeSpecInput make_ProductTimeAnchorSpecInput_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                         const OptDict_t& opt) {
        using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

        try {
            ProductTimeSpecInput input;
            input.marsYear                             = deductions::resolve_Year_opt(mars, par, opt);
            input.marsMonth                            = deductions::resolve_Month_opt(mars, par, opt);
            input.marsDate                             = deductions::resolve_Date_opt(mars, par, opt);
            input.marsTime                             = deductions::resolve_Time_opt(mars, par, opt);
            input.marsHdate                            = deductions::resolve_Hdate_opt(mars, par, opt);
            input.innerMostTypeOfStatisticalProcessing = tables::TypeOfStatisticalProcessing::Missing;

            MARS2GRIB_LOG_RESOLVE([&]() {
                return std::string{"`ProductTimeAnchorSpecInput` built from deductions: "} + input.to_json();
            }());

            return input;
        }
        catch (...) {
            std::throw_with_nested(
                Mars2GribModelException("Failed to build `ProductTimeAnchorSpecInput` from deduction outputs", Here()));
        }

        mars2gribUnreachable();
    }

    ///
    /// @brief Build the staged initialization components from reduced normalized input.
    ///
    /// This helper executes the anchor-only backend-model pipeline after a
    /// normalized input snapshot already exists:
    /// 1. classify anchor;
    /// 2. build anchor;
    /// 3. return the immutable member bundle used by the final constructor.
    ///
    /// @param[in] input Complete reduced normalized `ProductTimeSpecInput` snapshot.
    /// @return Complete staged component bundle for final member initialization.
    /// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
    ///         any classification or build failure, with the original cause
    ///         attached via `std::throw_with_nested`.
    ///
    static ProductTimeAnchorSpecComponents build_ProductTimeAnchorSpecComponents_or_throw(ProductTimeSpecInput input) {
        using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

        try {
            const anchor::ProductTimeSpecAnchorKind anchorType = anchor::classify_Anchor_or_throw(input);
            anchor::ProductTimeSpecAnchor anchor               = anchor::build_Anchor_or_throw(anchorType, input);

            ProductTimeAnchorSpecComponents result;
            result.anchorType = anchorType;
            result.anchor     = std::move(anchor);
            return result;
        }
        catch (...) {
            std::throw_with_nested(Mars2GribModelException(
                "Failed to build `ProductTimeAnchorSpec` staged components from normalized input", input.to_json(),
                Here()));
        }

        mars2gribUnreachable();
    }

    ///
    /// @brief Materialize the anchor-only model from an already-built
    ///        normalized input.
    ///
    /// @param[in] input Complete normalized anchor-only input snapshot.
    ///
    explicit ProductTimeAnchorSpec(ProductTimeSpecInput input) :
        ProductTimeAnchorSpec(build_ProductTimeAnchorSpecComponents_or_throw(std::move(input))) {}

    ///
    /// @brief Final private construction stage from staged components.
    ///
    /// @param[in] components Staged component bundle used for final member
    ///            initialization.
    ///
    explicit ProductTimeAnchorSpec(ProductTimeAnchorSpecComponents components) :
        anchorType_(components.anchorType), anchor_(std::move(components.anchor)) {}

    const anchor::ProductTimeSpecAnchorKind anchorType_;
    const anchor::ProductTimeSpecAnchor anchor_;
};

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
    /// layer and then delegates all classification, build, and final member
    /// initialization to private construction stages.
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
    ProductTimeSpec(tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing, const MarsDict_t& mars,
                    const ParDict_t& par, const OptDict_t& opt) :
        ProductTimeSpec(make_ProductTimeSpecInput_or_throw(innerMostTypeOfStatisticalProcessing, mars, par, opt)) {}

    /// @brief Return the resolved anchor classification.
    anchor::ProductTimeSpecAnchorKind anchorType() const noexcept { return anchorType_; }

    /// @brief Return the resolved shape classification.
    shape::ProductTimeSpecShapeKind shapeType() const noexcept { return shapeType_; }

    /// @brief Return the resolved domain classification.
    domain::ProductTimeSpecDomainKind domainType() const noexcept { return domainType_; }

    /// @brief Return the resolved anchor artifact.
    const anchor::ProductTimeSpecAnchor& anchor() const noexcept { return anchor_; }

    /// @brief Return the resolved absolute support domain.
    const domain::ProductTimeSpecDomain& domain() const noexcept { return domain_; }

    /// @brief Return the resolved canonical window sequence.
    const shape::ProductTimeSpecShape& windows() const noexcept { return windows_; }

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
            out << '{' << detail::jsonQuote_modelInput("anchorType") << ':'
                << detail::jsonQuote_modelInput(anchor::productTimeSpecAnchorTypeName(anchorType_)) << ','
                << detail::jsonQuote_modelInput("shapeType") << ':'
                << detail::jsonQuote_modelInput(shape::productTimeSpecShapeTypeName(shapeType_)) << ','
                << detail::jsonQuote_modelInput("domainType") << ':'
                << detail::jsonQuote_modelInput(domain::productTimeSpecDomainTypeName(domainType_)) << ','
                << detail::jsonQuote_modelInput("anchor") << ':' << anchor::productTimeSpecAnchorJson(anchor_) << ','
                << detail::jsonQuote_modelInput("domain") << ':' << domain::productTimeSpecDomainJson(domain_) << ','
                << detail::jsonQuote_modelInput("windows") << ':' << shape::productTimeSpecShapeJson(windows_) << '}';
            return out.str();
        }
        catch (...) {
            return std::string{"{\"error\":\"ProductTimeSpec::to_json() failed while building diagnostic context\"}"};
        }
    }

private:

    struct ProductTimeSpecComponents {
        anchor::ProductTimeSpecAnchorKind anchorType{anchor::ProductTimeSpecAnchorKind::ForecastAnalysis};
        shape::ProductTimeSpecShapeKind shapeType{shape::ProductTimeSpecShapeKind::InstantTimespanMissing};
        domain::ProductTimeSpecDomainKind domainType{domain::ProductTimeSpecDomainKind::ForecastDomain};
        anchor::ProductTimeSpecAnchor anchor{};
        domain::ProductTimeSpecDomain domain{};
        shape::ProductTimeSpecShape windows{};
    };

    ///
    /// @brief Build the staged initialization components from normalized input.
    ///
    /// This helper executes the backend-model ProductTimeSpec pipeline after a
    /// normalized input snapshot already exists:
    /// 1. classify anchor;
    /// 2. build anchor;
    /// 3. classify domain;
    /// 4. build domain;
    /// 5. classify shape;
    /// 6. build shape windows;
    /// 7. return the immutable member bundle used by the final constructor.
    ///
    /// @param[in] input Complete normalized ProductTimeSpec input snapshot.
    /// @return Complete staged component bundle for final member initialization.
    ///
    /// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
    ///         any classification or build failure, with the original cause
    ///         attached via `std::throw_with_nested`.
    ///
    static ProductTimeSpecComponents build_ProductTimeSpecComponents_or_throw(ProductTimeSpecInput input) {
        using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

        try {
            const anchor::ProductTimeSpecAnchorKind anchorType = anchor::classify_Anchor_or_throw(input);
            anchor::ProductTimeSpecAnchor anchor               = anchor::build_Anchor_or_throw(anchorType, input);
            const domain::ProductTimeSpecDomainKind domainType = domain::classify_Domain_or_throw(input);
            domain::ProductTimeSpecDomain domain            = domain::build_Domain_or_throw(domainType, input, anchor);
            const shape::ProductTimeSpecShapeKind shapeType = shape::classify_Shape_or_throw(input, domainType);
            shape::ProductTimeSpecShape windows;
            windows.values = shape::build_Shape_or_throw(shapeType, input, domain);

            ProductTimeSpecComponents result;
            result.anchorType = anchorType;
            result.shapeType  = shapeType;
            result.domainType = domainType;
            result.anchor     = std::move(anchor);
            result.domain     = std::move(domain);
            result.windows    = std::move(windows);
            return result;
        }
        catch (...) {
            std::throw_with_nested(Mars2GribModelException(
                "Failed to build `ProductTimeSpec` staged components from normalized input", input.to_json(), Here()));
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
        domainType_(components.domainType),
        anchor_(std::move(components.anchor)),
        domain_(std::move(components.domain)),
        windows_(std::move(components.windows)) {}

    const anchor::ProductTimeSpecAnchorKind anchorType_;
    const shape::ProductTimeSpecShapeKind shapeType_;
    const domain::ProductTimeSpecDomainKind domainType_;
    const anchor::ProductTimeSpecAnchor anchor_;
    const domain::ProductTimeSpecDomain domain_;
    const shape::ProductTimeSpecShape windows_;
};

}  // namespace metkit::mars2grib::backend::models::product_time_spec
