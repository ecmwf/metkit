/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


/**
 * @file ProductTimeSpecIncrementResolver.h
 * @brief Time-increment classification and increment-artifact construction.
 *
 * This header implements the ProductTimeSpec resolver stage that classifies the
 * increment semantics of the product and then constructs the corresponding
 * `ProductTimeSpecIncrement` artifact.
 *
 * It owns:
 *
 * - increment classification from explicit increment presence, shape kind,
 *   `class="ml"` special cases, and option-side policy;
 * - construction of explicit, defaulted, missing, and no-increment artifacts;
 * - extraction of the resolved innermost window range from the shape artifact;
 * - temporary default-increment derivation from the resolved window length.
 *
 * It does not own:
 *
 * - dictionary access or lexical parsing;
 * - time-anchor or product-shape construction;
 * - final canonical-window materialization.
 *
 * The implementation follows `productTimeSpecV3_final.md`, especially
 * Sections 3.3.3, 4.14, and 5.12. The current `DefaultedIncrement`
 * construction should be kept synchronized with the specification when that
 * section is updated.
 */

#pragma once


#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecResolverCommon.h"

namespace metkit::mars2grib::product_time_spec {
namespace resolver_detail {

/**
 * @brief Construct the zero-increment artifact used by instant products.
 *
 * @param shape Resolved shape artifact. Unused; kept for uniform dispatch.
 * @return `NoIncrement` artifact with the missing-increment sentinel.
 * @throws Nothing.
 */
template <class Input_t>
ProductTimeSpecIncrement constructNoIncrement(const Input_t&,
    const ProductTimeSpecShape& shape) {
    return ProductTimeSpecIncrement{ProductTimeDuration{tables::TimeUnit::Second, 0},
                                    TypeOfTimeIncrement::Missing,
                                    TimeIncrementKind::NoIncrement};
}

/**
 * @brief Return the resolved innermost real statistical window range.
 *
 * The source depends on the resolved shape kind:
 *
 * - `StandardSingleLoop` and `MultiLoop`: `shape.innerTimeRange`;
 * - `FakeDoubleLoopSingleLoop`: `shape.stattypeBlocks[0].timeRange`.
 *
 * `Instant` and `FromStartSingleLoop` are not eligible for this defaulting
 * path and therefore fail.
 *
 * @param input Normalized input snapshot.
 * @param shape Resolved shape artifact.
 * @return Reference to the resolved innermost window range.
 * @throws Mars2GribProductTimeSpecException if no eligible resolved inner range
 *         exists.
 */
template <class Input_t>
const ProductTimeDuration& resolvedInnerWindowRange(const Input_t& input,
    const ProductTimeSpecShape& shape) {
    switch (shape.shapeType) {
        case ProductTimeSpecShapeKind::StandardSingleLoop:
        case ProductTimeSpecShapeKind::MultiLoop:
            if (!shape.innerTimeRange) {
                fail(ProductTimeSpecStage::TimeIncrementConstruction,
                     "resolved shape is missing innerTimeRange",
                     input);
            }
            return *shape.innerTimeRange;

        case ProductTimeSpecShapeKind::FakeDoubleLoopSingleLoop:
            if (shape.stattypeBlocks.empty()) {
                fail(ProductTimeSpecStage::TimeIncrementConstruction,
                     "fakeDoubleLoop shape is missing stattypeBlocks",
                     input);
            }
            return shape.stattypeBlocks[0].timeRange;

        case ProductTimeSpecShapeKind::Instant:
            fail(ProductTimeSpecStage::TimeIncrementConstruction,
                 "instant shape has no real inner window range",
                 input);

        case ProductTimeSpecShapeKind::FromStartSingleLoop:
            fail(ProductTimeSpecStage::TimeIncrementConstruction,
                 "from-start shape must not receive a defaulted increment",
                 input);

        case ProductTimeSpecShapeKind::Count:
            fail(ProductTimeSpecStage::TimeIncrementConstruction,
                 "invalid shape classification sentinel",
                 input);
    }

    mars2gribUnreachable();
}


template <class Input_t>
tables::TypeOfTimeIntervals deriveTypeOfTimeIncrement( const Input_t& input ){

    std::string marsClass = input.marsClass();
    std::string marsStream = input.marsStream();
    std::string marsType = input.marsType();
    long marsParamId = input.marsParamId();

    // For the moment this is the default value, but here we need logic
    return tables::TypeOfTimeIntervals::SameStartTimeForecastIncremented;

};


/**
 * @brief Derive the temporary default increment from the resolved inner window.
 *
 * Current temporary rules are:
 *
 * - inner window `< 1h`  -> hard error;
 * - inner window `== 1h` -> `600` seconds;
 * - inner window `> 1h`  -> `3600` seconds.
 *
 * The decision is based on the already resolved inner window length carried by
 * the shape artifact.
 *
 * @param input Normalized input snapshot.
 * @param shape Resolved shape artifact.
 * @return Derived default increment in seconds.
 * @throws Mars2GribProductTimeSpecException if the resolved window is smaller
 *         than one hour or uses an unsupported unit for this rule.
 */
template <class Input_t>
long deriveDefaultTimeIncrementInSeconds(const Input_t& input,
    const ProductTimeSpecShape& shape) {
    const auto& innerRange = resolvedInnerWindowRange(input, shape);

    switch (innerRange.unit) {
        case tables::TimeUnit::Second:
            if (innerRange.length < 3600) {
                fail(ProductTimeSpecStage::TimeIncrementConstruction,
                     "defaulted increment requires resolved inner window length >= 1 hour",
                     input);
            }
            return innerRange.length == 3600 ? 600L : 3600L;

        case tables::TimeUnit::Hour:
            if (innerRange.length < 1) {
                fail(ProductTimeSpecStage::TimeIncrementConstruction,
                     "defaulted increment requires resolved inner window length >= 1 hour",
                     input);
            }
            return innerRange.length == 1 ? 600L : 3600L;

        case tables::TimeUnit::Day:
        case tables::TimeUnit::Month:
            return 3600L;

        default:
            fail(ProductTimeSpecStage::TimeIncrementConstruction,
                 "defaulted increment does not support this resolved inner window unit",
                 input);
    }

    mars2gribUnreachable();
}

/**
 * @brief Construct the explicit-increment artifact.
 *
 * A valid explicit increment requires:
 *
 * - a present positive source `timeIncrementInSeconds`;
 * - a non-missing policy `defaultTypeOfTimeIncrement`.
 *
 * The source seconds value is canonicalized to the coarsest exact elapsed
 * `ProductTimeDuration` representation.
 *
 * @param input Normalized input snapshot.
 * @param shape Resolved shape artifact. Unused; kept for uniform dispatch.
 * @return `ExplicitIncrement` artifact.
 * @throws Mars2GribProductTimeSpecException if the explicit source is missing or
 *         the policy type is missing.
 */
template <class Input_t>
ProductTimeSpecIncrement constructExplicitIncrement(const Input_t& input,
    const ProductTimeSpecShape& shape) {
    if (!input.timeIncrementInSeconds()) {
        fail(ProductTimeSpecStage::TimeIncrementConstruction,
             "ExplicitIncrement classification has no source increment",
             input);
    }

    auto typeOfTimeIncrement = deriveTypeOfTimeIncrement(input);
    if (typeOfTimeIncrement == TypeOfTimeIncrement::Missing) {
        fail(ProductTimeSpecStage::TimeIncrementConstruction,
             "a real explicit increment requires non-missing typeOfTimeIncrement",
             input);
    }
    return ProductTimeSpecIncrement{
        canonicalElapsedDuration(*input.timeIncrementInSeconds()),
        typeOfTimeIncrement,
        TimeIncrementKind::ExplicitIncrement};
}

/**
 * @brief Construct the defaulted-increment artifact from resolved shape data.
 *
 * The defaulted increment is materialized during construction, not left as a
 * deferred marker. The current implementation derives the value from the
 * resolved innermost window length and requires a non-missing policy
 * `defaultTypeOfTimeIncrement`.
 *
 * @param input Normalized input snapshot.
 * @param shape Resolved shape artifact.
 * @return `DefaultedIncrement` artifact.
 * @throws Mars2GribProductTimeSpecException if the policy type is missing or no
 *         valid derived default increment exists.
 */
template <class Input_t>
ProductTimeSpecIncrement constructDefaultedIncrement(const Input_t& input,
    const ProductTimeSpecShape& shape) {

    const long defaultIncrementInSeconds =
        deriveDefaultTimeIncrementInSeconds(input, shape);


    auto typeOfTimeIncrement = deriveTypeOfTimeIncrement(input);
    if (typeOfTimeIncrement == TypeOfTimeIncrement::Missing) {
        fail(ProductTimeSpecStage::TimeIncrementConstruction,
             "a real explicit increment requires non-missing typeOfTimeIncrement",
             input);
    }

    return ProductTimeSpecIncrement{
        canonicalElapsedDuration(defaultIncrementInSeconds),
        typeOfTimeIncrement,
        TimeIncrementKind::DefaultedIncrement};
}

/**
 * @brief Construct the AIFS pure-missing increment sentinel artifact.
 *
 * @return `AifsPureMissingIncrement` artifact with a zero-second missing
 *         increment sentinel.
 * @throws Nothing.
 */
template <class Input_t>
ProductTimeSpecIncrement constructAifsMissingIncrement(const Input_t&,
    const ProductTimeSpecShape&) {
    return ProductTimeSpecIncrement{ProductTimeDuration{tables::TimeUnit::Second, 0},
                                    TypeOfTimeIncrement::Missing,
                                    TimeIncrementKind::AifsPureMissingIncrement};
}

}  // namespace resolver_detail

/**
 * @brief Classify the time-increment semantics of the product.
 *
 * Classification depends on:
 *
 * - whether an explicit positive source increment is present;
 * - whether the shape is instant or from-start;
 * - whether the product is the `class="ml"` single-real-window AIFS case;
 * - whether defaulting is allowed by option-side policy.
 *
 * This stage remains intentionally coarse: for defaulted increments it decides
 * eligibility, while the concrete default value is materialized later during
 * increment construction from the resolved shape artifact.
 *
 * @param input Normalized input snapshot.
 * @param shapeType Valid shape classification.
 * @return Valid `TimeIncrementKind` classification.
 * @throws Mars2GribProductTimeSpecException on incompatible or missing
 *         increment states.
 */
template <class Input_t>
TimeIncrementKind classify_TimeIncrement_or_throw(
    const Input_t& input,
    ProductTimeSpecShapeKind shapeType) {
    const bool hasExplicit = input.timeIncrementInSeconds().has_value();
    const std::size_t realCount =
        realStatisticalWindowCount(shapeType, input.stattypeBlocks().size());

    if (shapeType == ProductTimeSpecShapeKind::Instant) {
        if (hasExplicit && !input.options().allowRedundantTimeIncrement) {
            resolver_detail::fail(ProductTimeSpecStage::TimeIncrementClassification,
                                  "instant product carries an explicit redundant increment",
                                  input);
        }
        return TimeIncrementKind::NoIncrement;
    }

    if (input.marsClass() == "ml" && realCount == 1) {
        if (hasExplicit && !input.options().allowRedundantTimeIncrement) {
            resolver_detail::fail(ProductTimeSpecStage::TimeIncrementClassification,
                                  "AIFS-pure single-window product carries an explicit redundant increment",
                                  input);
        }
        return TimeIncrementKind::AifsPureMissingIncrement;
    }

    if (hasExplicit) {
        return TimeIncrementKind::ExplicitIncrement;
    }

    if (input.marsClass() != "ml" &&
        shapeType == ProductTimeSpecShapeKind::FromStartSingleLoop) {
        resolver_detail::fail(ProductTimeSpecStage::TimeIncrementClassification,
                              "non-ml from-start product requires an explicit increment",
                              input);
    }

    if (input.marsClass() != "ml" &&
        input.options().allowDefaultTimeIncrementInSeconds) {
        return TimeIncrementKind::DefaultedIncrement;
    }

    resolver_detail::fail(ProductTimeSpecStage::TimeIncrementClassification,
                          "statistical product has no usable increment",
                          input);

    mars2gribUnreachable();
}

/**
 * @brief Construct one increment artifact from a valid increment classification.
 *
 * The function dispatches to one specialized increment constructor indexed by
 * the valid `TimeIncrementKind` classification. The resolved shape artifact is
 * passed through so `DefaultedIncrement` construction can inspect the resolved
 * inner window.
 *
 * @param input Normalized input snapshot.
 * @param shape Resolved shape artifact.
 * @param incrementType Valid increment classification.
 * @return Fully resolved `ProductTimeSpecIncrement` artifact.
 * @throws Mars2GribProductTimeSpecException on dispatch or construction
 *         failures.
 */
template <class Input_t>
ProductTimeSpecIncrement construct_ProductTimeSpecIncrement_or_throw(
    const Input_t& input,
    const ProductTimeSpecShape& shape,
    TimeIncrementKind incrementType) {
    using Builder = ProductTimeSpecIncrement (*)(const Input_t&, const ProductTimeSpecShape&);
    static const std::array<Builder, static_cast<std::size_t>(TimeIncrementKind::Count)>
        builders{
            &resolver_detail::constructNoIncrement<Input_t>,
            &resolver_detail::constructExplicitIncrement<Input_t>,
            &resolver_detail::constructDefaultedIncrement<Input_t>,
            &resolver_detail::constructAifsMissingIncrement<Input_t>};

    const auto index = static_cast<std::size_t>(incrementType);
    if (index >= builders.size()) {
        resolver_detail::fail(ProductTimeSpecStage::TimeIncrementConstruction,
                              "invalid increment dispatch index",
                              input);
    }
    try {
        return builders[index](input, shape);
    } catch (const Mars2GribProductTimeSpecException&) {
        throw;
    } catch (const std::exception& e) {
        resolver_detail::fail(ProductTimeSpecStage::TimeIncrementConstruction,
                              e.what(),
                              input);
    }
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::product_time_spec
