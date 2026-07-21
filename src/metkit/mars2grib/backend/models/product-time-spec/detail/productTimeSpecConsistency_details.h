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
/// @file productTimeSpecConsistency_details.h
/// @brief Internal helpers for final ProductTimeSpec consistency validation.
///

#pragma once

#include <cstddef>

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDomain.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecWindows.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecTimeUtils.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::detail {

inline bool equalProductTimeSpecDuration(const deductions::TimeDuration& lhs,
                                         const deductions::TimeDuration& rhs) {
    return lhs.length == rhs.length && lhs.unit == rhs.unit;
}

template <class Windows_t>
std::size_t countRealProductTimeSpecWindows(const Windows_t& windows,
                                            ProductTimeSpecShapeKind shapeType) {
    return shapeType == ProductTimeSpecShapeKind::Instant ? 0 : windows.values.size();
}

template <class Input_t>
void checkProductTimeSpecConsistencyAnchor_or_throw(const Input_t& input,
                                                    TimeAnchorKind anchorType,
                                                    const ProductTimeSpecAnchor& anchor) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (anchor.anchorType != anchorType) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation received inconsistent anchor classification and anchor artifact",
            input.to_json(),
            Here());
    }

    if (!(anchor.labelDateTime <= anchor.initialConditionsDateTime &&
          anchor.initialConditionsDateTime <= anchor.referenceDateTime)) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected invalid anchor datetime ordering",
            input.to_json(),
            Here());
    }
}

template <class Input_t>
void checkProductTimeSpecConsistencyDomainOrdering_or_throw(const Input_t& input,
                                                            const ProductTimeSpecDomain& domain) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (domain.domainStartDateTime > domain.domainEndDateTime) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected `domainStartDateTime > domainEndDateTime`",
            input.to_json(),
            Here());
    }
}

template <class Input_t>
void checkProductTimeSpecConsistencyWindowCardinality_or_throw(const Input_t& input,
                                                               ProductTimeSpecShapeKind shapeType,
                                                               const ProductTimeSpecWindows& windows) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    const std::size_t expected =
        shapeType == ProductTimeSpecShapeKind::Instant
            ? 1
            : shapeType == ProductTimeSpecShapeKind::MultiLoop
                  ? (input.stattype.has_value() ? input.stattype->size() + 1 : 1)
                  : shapeType == ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop ? 2 : 1;

    if (windows.values.size() != expected) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected invalid shape-specific window cardinality",
            input.to_json(),
            Here());
    }
}

template <class Input_t>
void checkProductTimeSpecConsistencyInstant_or_throw(const Input_t& input,
                                                     TimeIncrementKind incrementType,
                                                     const ProductTimeSpecDomain& domain,
                                                     const ProductTimeSpecWindows& windows) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    const auto zero = deductions::TimeDuration{0, tables::TimeUnit::Second};
    const auto& window = windows.values[0];

    if (domain.domainStartDateTime != domain.domainEndDateTime ||
        window.typeOfStatisticalProcessing != tables::TypeOfStatisticalProcessing::Missing ||
        !equalProductTimeSpecDuration(window.timeRange, zero) ||
        !equalProductTimeSpecDuration(window.timeIncrement, zero) ||
        incrementType != TimeIncrementKind::NoIncrement) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected an invalid instant placeholder invariant",
            input.to_json(),
            Here());
    }
}

template <class Input_t>
void checkProductTimeSpecConsistencyOutermostSupport_or_throw(const Input_t& input,
                                                              const ProductTimeSpecAnchor& anchor,
                                                              const ProductTimeSpecDomain& domain,
                                                              const ProductTimeSpecWindows& windows) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    try {
        const eckit::DateTime expectedStart = subtractProductTimeSpecDuration_or_throw(
            domain.domainEndDateTime,
            windows.values[0].timeRange);

        if (expectedStart != domain.domainStartDateTime) {
            throw Mars2GribModelException(
                "Final ProductTimeSpec validation detected a mismatch between the domain start and the outermost canonical range",
                input.to_json(),
                Here());
        }
    } catch (const Mars2GribModelException&) {
        std::throw_with_nested(Mars2GribModelException(
            "Final ProductTimeSpec validation failed while recomputing the outer support start",
            input.to_json(),
            Here()));
    } catch (const std::exception& e) {
        throw Mars2GribModelException(
            std::string("Final ProductTimeSpec validation failed while recomputing the outer support start: ") + e.what(),
            input.to_json(),
            Here());
    }

    if (domain.domainStartDateTime < anchor.referenceDateTime) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected support beginning before `referenceDateTime`",
            input.to_json(),
            Here());
    }
}

template <class Input_t>
void checkProductTimeSpecConsistencyFromStart_or_throw(const Input_t& input,
                                                       const ProductTimeSpecAnchor& anchor,
                                                       const ProductTimeSpecDomain& domain) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (domain.domainStartDateTime != anchor.referenceDateTime) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected a from-start domain not beginning at `referenceDateTime`",
            input.to_json(),
            Here());
    }
}

template <class Input_t>
void checkProductTimeSpecConsistencyAifsMissingIncrement_or_throw(const Input_t& input,
                                                                  ProductTimeSpecShapeKind shapeType,
                                                                  const ProductTimeSpecWindows& windows) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    const auto zero = deductions::TimeDuration{0, tables::TimeUnit::Second};
    const auto realWindowCount = countRealProductTimeSpecWindows(windows, shapeType);

    if (input.marsClass != "ml" || realWindowCount != 1 ||
        !equalProductTimeSpecDuration(windows.values.back().timeIncrement, zero)) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected an invalid AIFS pure-missing increment sentinel state",
            input.to_json(),
            Here());
    }
}

template <class Input_t>
void checkProductTimeSpecConsistencyFakeSingleLoopDoubleLoop_or_throw(const Input_t& input,
                                                                      const ProductTimeSpecWindows& windows) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (windows.values.size() != 2) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected an invalid fake-single-loop/double-loop window count",
            input.to_json(),
            Here());
    }

    if (windows.values[0].typeOfStatisticalProcessing != tables::TypeOfStatisticalProcessing::IndexProcessing) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected an invalid fake-single-loop/double-loop outer processing type",
            input.to_json(),
            Here());
    }

    if (windows.values[1].typeOfStatisticalProcessing != input.innerMostTypeOfStatisticalProcessing) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected an invalid fake-single-loop/double-loop inner processing type",
            input.to_json(),
            Here());
    }

    if (!equalProductTimeSpecDuration(windows.values[0].timeRange, windows.values[1].timeRange) ||
        !equalProductTimeSpecDuration(windows.values[0].timeIncrement, windows.values[1].timeIncrement)) {
        throw Mars2GribModelException(
            "Final ProductTimeSpec validation detected inconsistent fake-single-loop/double-loop windows",
            input.to_json(),
            Here());
    }
}

}  // namespace metkit::mars2grib::backend::models::detail
