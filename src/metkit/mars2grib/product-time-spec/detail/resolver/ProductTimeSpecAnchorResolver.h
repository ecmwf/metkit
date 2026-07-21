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
 * @file ProductTimeSpecAnchorResolver.h
 * @brief Time-anchor classification and anchor-artifact construction.
 *
 * This header implements the ProductTimeSpec resolver stage that determines how
 * the three ordered anchor datetimes are sourced and then constructs the
 * corresponding `ProductTimeSpecAnchor` artifact.
 *
 * It owns:
 *
 * - time-anchor classification from normalized direct-source presence;
 * - hierarchical datetime inheritance through the anchor construction model;
 * - ordering checks on the resolved `label`, `initialConditions`, and
 *   `reference` datetimes;
 * - dispatch to the specialized anchor constructors.
 *
 * It does not own:
 *
 * - dictionary access or lexical parsing;
 * - shape or increment logic;
 * - final ProductTimeSpec canonicalization.
 *
 * The implementation follows `productTimeSpecV3_final.md`, especially
 * Sections 3.3.1, 4.12, and 5.9.
 */

#pragma once


#include "eckit/types/Date.h"
#include "eckit/types/DateTime.h"
#include "eckit/types/Time.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/product-time-spec/detail/resolver/ProductTimeSpecResolverCommon.h"

namespace metkit::mars2grib::product_time_spec {
namespace resolver_detail {

/**
 * @brief Build the direct label datetime candidate used by anchor inheritance.
 *
 * Direct-source precedence matches the anchor model:
 *
 * - `date` plus `time.value_or(defaultMarsTime)` when `date` is present;
 * - otherwise `hdate` plus `defaultMarsTime`;
 * - otherwise the first day of `year` / `month` plus `defaultMarsTime`.
 *
 * The surrounding classifier guarantees that at least one direct source exists.
 *
 * @param input Normalized input snapshot.
 * @return Direct label candidate datetime.
 * @throws Standard datetime construction failures may propagate.
 */
template <class Input_t>
eckit::DateTime directLabel(const Input_t& input) {
    if (input.marsDate()) {
        return eckit::DateTime(*input.marsDate(), input.marsTime().value_or(defaultMarsTime));
    }
    if (input.marsHdate()) {
        return eckit::DateTime(*input.marsHdate(), defaultMarsTime);
    }
    return eckit::DateTime(eckit::Date(*input.marsYear(), *input.marsMonth(), 1),
                           defaultMarsTime);
}

/**
 * @brief Construct one fully resolved anchor artifact for a known anchor kind.
 *
 * The function applies the hierarchical inheritance model described by the
 * specification and then checks the required ordering:
 *
 * `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
 *
 * @param input Normalized input snapshot.
 * @param kind Already-classified anchor kind.
 * @return Fully resolved anchor artifact.
 * @throws Mars2GribProductTimeSpecException if the resolved datetimes violate
 *         the required ordering.
 */
template <class Input_t>
ProductTimeSpecAnchor constructAnchorCommon(const Input_t& input, TimeAnchorKind kind) {
    const eckit::DateTime label = directLabel(input);
    const eckit::DateTime initial = input.marsHdate()
                                            ? eckit::DateTime(*input.marsHdate(), defaultMarsTime)
                                            : label;
    const eckit::DateTime reference = input.marsYear()
                                              ? eckit::DateTime(
                                                    eckit::Date(*input.marsYear(), *input.marsMonth(), 1),
                                                    defaultMarsTime)
                                              : initial;

    if (label > initial || initial > reference) {
        fail(ProductTimeSpecStage::TimeAnchorConstruction,
             "required ordering labelDateTime <= initialConditionsDateTime <= referenceDateTime is violated",
             input);
    }

    return ProductTimeSpecAnchor{label, initial, reference, kind};
}

/** @brief Construct the `LabelOnly` anchor artifact. */
template <class Input_t>
ProductTimeSpecAnchor constructLabelOnly(const Input_t& input) {
    return constructAnchorCommon(input, TimeAnchorKind::LabelOnly);
}

/** @brief Construct the `Hindcast` anchor artifact. */
template <class Input_t>
ProductTimeSpecAnchor constructHindcast(const Input_t& input) {
    return constructAnchorCommon(input, TimeAnchorKind::Hindcast);
}

/** @brief Construct the `ForecastAnchor` anchor artifact. */
template <class Input_t>
ProductTimeSpecAnchor constructForecastAnchor(const Input_t& input) {
    return constructAnchorCommon(input, TimeAnchorKind::ForecastAnchor);
}

/** @brief Construct the `HindcastForecastAnchor` anchor artifact. */
template <class Input_t>
ProductTimeSpecAnchor constructHindcastForecastAnchor(const Input_t& input) {
    return constructAnchorCommon(input, TimeAnchorKind::HindcastForecastAnchor);
}

}  // namespace resolver_detail

/**
 * @brief Classify the direct-anchor source regime.
 *
 * Classification depends only on the presence of the two special direct anchor
 * sources:
 *
 * - direct `hdate`;
 * - direct `year` / `month`.
 *
 * It also rejects invalid direct-source states such as:
 *
 * - `time` present without `date`;
 * - complete absence of every direct anchor source.
 *
 * @param input Normalized input snapshot.
 * @return Valid `TimeAnchorKind` classification.
 * @throws Mars2GribProductTimeSpecException on invalid direct-source states.
 */
template <class Input_t>
TimeAnchorKind classify_TimeAnchor_or_throw(const Input_t& input) {
    if (input.marsTime() && !input.marsDate()) {
        resolver_detail::fail(ProductTimeSpecStage::TimeAnchorClassification,
                              "`time` is present without `date`",
                              input);
    }
    if (!input.marsDate() && !input.marsHdate() && !input.marsYear()) {
        resolver_detail::fail(ProductTimeSpecStage::TimeAnchorClassification,
                              "no direct temporal anchor source is present",
                              input);
    }

    const bool hasHdate = input.marsHdate().has_value();
    const bool hasReferenceAnchor = input.marsYear().has_value();
    if (!hasHdate && !hasReferenceAnchor) return TimeAnchorKind::LabelOnly;
    if (hasHdate && !hasReferenceAnchor) return TimeAnchorKind::Hindcast;
    if (!hasHdate && hasReferenceAnchor) return TimeAnchorKind::ForecastAnchor;
    return TimeAnchorKind::HindcastForecastAnchor;
}

/**
 * @brief Construct one anchor artifact from a valid anchor classification.
 *
 * The function dispatches to one specialized constructor indexed by the valid
 * `TimeAnchorKind` classification.
 *
 * @param input Normalized input snapshot.
 * @param anchorType Valid anchor classification.
 * @return Fully resolved `ProductTimeSpecAnchor` artifact.
 * @throws Mars2GribProductTimeSpecException on dispatch or construction
 *         failures.
 */
template <class Input_t>
ProductTimeSpecAnchor construct_ProductTimeSpecAnchor_or_throw(
    const Input_t& input,
    TimeAnchorKind anchorType) {
    using Builder = ProductTimeSpecAnchor (*)(const Input_t&);
    static const std::array<Builder, static_cast<std::size_t>(TimeAnchorKind::Count)>
        builders{
            &resolver_detail::constructLabelOnly<Input_t>,
            &resolver_detail::constructHindcast<Input_t>,
            &resolver_detail::constructForecastAnchor<Input_t>,
            &resolver_detail::constructHindcastForecastAnchor<Input_t>};

    const auto index = static_cast<std::size_t>(anchorType);
    if (index >= builders.size()) {
        resolver_detail::fail(ProductTimeSpecStage::TimeAnchorConstruction,
                              "invalid anchor dispatch index",
                              input);
    }
    try {
        return builders[index](input);
    } catch (const Mars2GribProductTimeSpecException&) {
        throw;
    } catch (const std::exception& e) {
        resolver_detail::fail(ProductTimeSpecStage::TimeAnchorConstruction,
                              e.what(),
                              input);
    }

    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::product_time_spec
