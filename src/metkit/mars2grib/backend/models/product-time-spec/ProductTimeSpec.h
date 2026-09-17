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

#include "metkit/mars2grib/utils/profiling/Profiling.h"

#include <string>
#include <utility>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/domains/DomainDataTypes.h"
#include "metkit/mars2grib/backend/models/product-time-spec/shapes/ShapeDataTypes.h"


#include "metkit/mars2grib/backend/models/product-time-spec/anchors/AnchorRegistry.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ShapeNormalization.h"
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
    template <class MarsDict_t, class ParDict_t, class OptDict_t, class Cntx_t>
    ProductTimeAnchorSpec(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, Cntx_t& cntx) :
        ProductTimeAnchorSpec(
            make_ProductTimeAnchorSpecInput_or_throw(
                mars, par, opt, metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
            metkit::mars2grib::utils::profiling::callSite(cntx, Here())) {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    }

    /// @brief Return the resolved anchor classification.
    template <class Cntx_t>
    anchor::ProductTimeSpecAnchorKind anchorType(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            anchor::ProductTimeSpecAnchorKind result = anchorType_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    /// @brief Return the resolved anchor artifact.
    template <class Cntx_t>
    const anchor::ProductTimeSpecAnchor& anchor(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            const anchor::ProductTimeSpecAnchor& result = anchor_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

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
    template <class Cntx_t>
    std::string to_json(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        try {
            std::ostringstream out;
            out << '{' << detail::jsonQuote_modelInput("anchorType", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
                << detail::jsonQuote_modelInput(anchor::productTimeSpecAnchorTypeName(anchorType_), metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
                << detail::jsonQuote_modelInput("anchor", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << anchor::productTimeSpecAnchorJson(anchor_, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << '}';
            {
                std::string result = out.str();
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }
        catch (...) {
            {
                std::string result = std::string{
                "{\"error\":\"ProductTimeAnchorSpec::to_json() failed while building diagnostic context\"}"};
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
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
    template <class MarsDict_t, class ParDict_t, class OptDict_t, class Cntx_t>
    static ProductTimeSpecInput make_ProductTimeAnchorSpecInput_or_throw(const MarsDict_t& mars, const ParDict_t& par,
                                                                         const OptDict_t& opt, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

        try {
            ProductTimeSpecInput input;
            input.marsYear                             = deductions::resolve_Year_opt(mars, par, opt, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            input.marsMonth                            = deductions::resolve_Month_opt(mars, par, opt, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            input.marsDate                             = deductions::resolve_Date_opt(mars, par, opt, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            input.marsTime                             = deductions::resolve_Time_opt(mars, par, opt, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            input.marsHdate                            = deductions::resolve_Hdate_opt(mars, par, opt, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            input.innerMostTypeOfStatisticalProcessing = tables::TypeOfStatisticalProcessing::Missing;

            MARS2GRIB_LOG_RESOLVE([&]() {
                return std::string{"`ProductTimeAnchorSpecInput` built from deductions: "} + input.to_json(cntx);
            }());

            {
                ProductTimeSpecInput result = input;
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
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
template <class Cntx_t>
    static ProductTimeAnchorSpecComponents build_ProductTimeAnchorSpecComponents_or_throw(ProductTimeSpecInput input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

        try {
            const anchor::ProductTimeSpecAnchorKind anchorType = anchor::classify_Anchor_or_throw(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            ProductTimeSpecClassification classification;
            classification.anchorType            = anchorType;
            anchor::ProductTimeSpecAnchor anchor = anchor::build_Anchor_or_throw(anchorType, input, classification, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

            ProductTimeAnchorSpecComponents result;
            result.anchorType = anchorType;
            result.anchor     = std::move(anchor);
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        catch (...) {
            std::throw_with_nested(Mars2GribModelException(
                "Failed to build `ProductTimeAnchorSpec` staged components from normalized input", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
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
    template <class Cntx_t>
    explicit ProductTimeAnchorSpec(ProductTimeSpecInput input, Cntx_t& cntx) :
        ProductTimeAnchorSpec(
            build_ProductTimeAnchorSpecComponents_or_throw(
                std::move(input), metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
            metkit::mars2grib::utils::profiling::callSite(cntx, Here())) {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    }

    ///
    /// @brief Final private construction stage from staged components.
    ///
    /// @param[in] components Staged component bundle used for final member
    ///            initialization.
    ///
    template <class Cntx_t>
    explicit ProductTimeAnchorSpec(ProductTimeAnchorSpecComponents components, Cntx_t& cntx) :
        anchorType_(components.anchorType), anchor_(std::move(components.anchor)) {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    }

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
    template <class MarsDict_t, class ParDict_t, class OptDict_t, class Cntx_t>
    ProductTimeSpec(tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing, const MarsDict_t& mars,
                    const ParDict_t& par, const OptDict_t& opt, Cntx_t& cntx) :
        ProductTimeSpec(
            make_ProductTimeSpecInput_or_throw(innerMostTypeOfStatisticalProcessing, mars, par, opt,
                                               metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
            metkit::mars2grib::utils::profiling::callSite(cntx, Here())) {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    }

    /// @brief Return the resolved anchor classification.
    template <class Cntx_t>
    anchor::ProductTimeSpecAnchorKind anchorType(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            anchor::ProductTimeSpecAnchorKind result = anchorType_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    /// @brief Return the resolved shape classification.
    template <class Cntx_t>
    shape::ProductTimeSpecShapeKind shapeType(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            shape::ProductTimeSpecShapeKind result = shapeType_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    /// @brief Return the resolved domain classification.
    template <class Cntx_t>
    domain::ProductTimeSpecDomainKind domainType(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            domain::ProductTimeSpecDomainKind result = domainType_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    /// @brief Return the resolved anchor artifact.
    template <class Cntx_t>
    const anchor::ProductTimeSpecAnchor& anchor(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            const anchor::ProductTimeSpecAnchor& result = anchor_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    /// @brief Return the resolved absolute support domain.
    template <class Cntx_t>
    const domain::ProductTimeSpecDomain& domain(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            const domain::ProductTimeSpecDomain& result = domain_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

    /// @brief Return the resolved canonical window sequence.
    template <class Cntx_t>
    const shape::ProductTimeSpecShape& windows(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        {
            const shape::ProductTimeSpecShape& result = windows_;
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
    }

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
    template <class Cntx_t>
    std::string to_json(Cntx_t& cntx) const noexcept {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        try {
            std::ostringstream out;
            out << '{' << detail::jsonQuote_modelInput("anchorType", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
                << detail::jsonQuote_modelInput(anchor::productTimeSpecAnchorTypeName(anchorType_), metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
                << detail::jsonQuote_modelInput("shapeType", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
                << detail::jsonQuote_modelInput(shape::productTimeSpecShapeTypeName(shapeType_), metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
                << detail::jsonQuote_modelInput("domainType", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':'
                << detail::jsonQuote_modelInput(domain::productTimeSpecDomainTypeName(domainType_), metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
                << detail::jsonQuote_modelInput("anchor", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << anchor::productTimeSpecAnchorJson(anchor_, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
                << detail::jsonQuote_modelInput("domain", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << domain::productTimeSpecDomainJson(domain_, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ','
                << detail::jsonQuote_modelInput("windows", metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << ':' << shape::productTimeSpecShapeJson(windows_, metkit::mars2grib::utils::profiling::callSite(cntx, Here())) << '}';
            {
                std::string result = out.str();
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }
        catch (...) {
            {
                std::string result = std::string{"{\"error\":\"ProductTimeSpec::to_json() failed while building diagnostic context\"}"};
                metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }
        }
    }

private:

    struct ProductTimeSpecComponents {
        anchor::ProductTimeSpecAnchorKind anchorType{anchor::ProductTimeSpecAnchorKind::ForecastAnalysis};
        shape::ProductTimeSpecShapeKind shapeType{shape::ProductTimeSpecShapeKind::Instant};
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
    /// 2. classify shape;
    /// 3. classify domain;
    /// 4. assemble the classification bundle;
    /// 5. build anchor;
    /// 6. build the stage-1 shape;
    /// 7. build domain;
    /// 8. build the final shape;
    /// 9. return the immutable member bundle used by the final constructor.
    ///
    /// @param[in] input Complete normalized ProductTimeSpec input snapshot.
    /// @return Complete staged component bundle for final member initialization.
    ///
    /// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException on
    ///         any classification or build failure, with the original cause
    ///         attached via `std::throw_with_nested`.
    ///
template <class Cntx_t>
    static ProductTimeSpecComponents build_ProductTimeSpecComponents_or_throw(ProductTimeSpecInput input, Cntx_t& cntx) {
    metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

        try {
            const anchor::ProductTimeSpecAnchorKind anchorType = anchor::classify_Anchor_or_throw(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            const domain::ProductTimeSpecDomainKind domainType = domain::classify_Domain_or_throw(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            const shape::ProductTimeSpecShapeKind shapeType    = shape::classify_Shape_or_throw(input, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

            ProductTimeSpecClassification classification;
            classification.anchorType = anchorType;
            classification.shapeType  = shapeType;
            classification.domainType = domainType;

            anchor::ProductTimeSpecAnchor anchor = anchor::build_Anchor_or_throw(anchorType, input, classification, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            const shape::ProductTimeSpecOuterTimeRange outerTimeRange =
                shape::build_ShapeOuterTimeRange_or_throw(shapeType, input, classification, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            domain::ProductTimeSpecDomain domain =
                domain::build_Domain_or_throw(domainType, input, classification, anchor, outerTimeRange, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));
            const shape::ProductTimeSpecShape rawWindows =
                shape::build_ShapeWindows_or_throw(shapeType, input, classification, anchor, outerTimeRange, domain, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

            const shape::ProductTimeSpecShape normalisedWindows =
                detail::normalizeShape_or_throw(input, domain, rawWindows, metkit::mars2grib::utils::profiling::callSite(cntx, Here()));

            ProductTimeSpecComponents result;
            result.anchorType = anchorType;
            result.shapeType  = shapeType;
            result.domainType = domainType;
            result.anchor     = std::move(anchor);
            result.domain     = std::move(domain);
            result.windows    = std::move(normalisedWindows);
            metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        catch (...) {
            std::throw_with_nested(Mars2GribModelException(
                "Failed to build `ProductTimeSpec` staged components from normalized input", input.to_json(metkit::mars2grib::utils::profiling::callSite(cntx, Here())), Here()));
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
    template <class Cntx_t>
    explicit ProductTimeSpec(ProductTimeSpecInput input, Cntx_t& cntx) :
        ProductTimeSpec(
            build_ProductTimeSpecComponents_or_throw(
                std::move(input), metkit::mars2grib::utils::profiling::callSite(cntx, Here())),
            metkit::mars2grib::utils::profiling::callSite(cntx, Here())) {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    }

    ///
    /// @brief Final private construction stage from staged components.
    ///
    /// This constructor is the single point at which immutable members are
    /// initialized from the fully resolved staged component bundle.
    ///
    /// @param[in] components Staged component bundle used for final member
    ///            initialization.
    ///
    template <class Cntx_t>
    explicit ProductTimeSpec(ProductTimeSpecComponents components, Cntx_t& cntx) :
        anchorType_(components.anchorType),
        shapeType_(components.shapeType),
        domainType_(components.domainType),
        anchor_(std::move(components.anchor)),
        domain_(std::move(components.domain)),
        windows_(std::move(components.windows)) {
        metkit::mars2grib::utils::profiling::profileEnterFunction(cntx, Here());
        metkit::mars2grib::utils::profiling::profileExitFunction(cntx, Here());
    }

    const anchor::ProductTimeSpecAnchorKind anchorType_;
    const shape::ProductTimeSpecShapeKind shapeType_;
    const domain::ProductTimeSpecDomainKind domainType_;
    const anchor::ProductTimeSpecAnchor anchor_;
    const domain::ProductTimeSpecDomain domain_;
    const shape::ProductTimeSpecShape windows_;
};

}  // namespace metkit::mars2grib::backend::models::product_time_spec
