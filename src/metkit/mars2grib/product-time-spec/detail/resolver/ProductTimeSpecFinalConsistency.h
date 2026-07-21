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
 * @file ProductTimeSpecFinalConsistency.h
 * @brief Final whole-object invariant checks for canonical ProductTimeSpec.
 */

#pragma once


#include "metkit/mars2grib/product-time-spec/ProductTimeSpec.h"
#include "metkit/mars2grib/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::product_time_spec {

/**
 * @brief Validate final whole-object ProductTimeSpec invariants.
 *
 * This function rechecks invariants that are meaningful only on the fully
 * materialized canonical object, including:
 *
 * - anchor ordering;
 * - support interval ordering;
 * - canonical range cardinality;
 * - absolute support consistency with the outermost canonical range;
 * - from-start support beginning at `referenceDateTime`;
 * - AIFS missing-increment sentinel invariants;
 * - agreement between frontend and final-IR real-window counts.
 *
 * @param input Normalized input snapshot.
 * @param classification Valid classification triple.
 * @param spec Final canonical ProductTimeSpec candidate.
 * @throws Mars2GribProductTimeSpecException if any whole-object invariant is
 *         violated.
 */
template <class Input_t>
void check_FinalConsistency_or_throw(
    const Input_t& input,
    const ProductTimeSpecClassification& classification,
    const ProductTimeSpec& spec) {
    const auto fail = [&](const std::string& reason) -> void {
        resolver_detail::fail(ProductTimeSpecStage::FinalConsistencyCheck,
                              reason,
                              input,
                              classification,
                              {},
                              spec.to_json());
    };

    if (!(spec.labelDateTime() <= spec.initialConditionsDateTime() &&
          spec.initialConditionsDateTime() <= spec.referenceDateTime())) {
        fail("anchor ordering invariant is violated");
    }
    if (spec.windowStartDateTime() > spec.windowEndDateTime()) {
        fail("windowStartDateTime is after windowEndDateTime");
    }
    if (spec.numberOfTimeRanges() < 1) {
        fail("canonical IR contains no time ranges");
    }
    if (spec.kind() != ProductTimeSpecKind::Instant &&
        spec.windowStartDateTime() < spec.referenceDateTime()) {
        fail("statistical support begins before referenceDateTime");
    }

    const std::size_t expectedCardinality =
        classification.shapeType == ProductTimeSpecShapeKind::MultiLoop
            ? input.stattypeBlocks().size() + 1
            : 1;
    if (spec.numberOfTimeRanges() != expectedCardinality) {
        fail("shape-specific canonical range cardinality is invalid");
    }

    if (spec.kind() == ProductTimeSpecKind::Instant) {
        const auto& window = spec[0];
        if (spec.windowStartDateTime() != spec.windowEndDateTime() ||
            window.typeOfStatisticalProcessing !=
                tables::TypeOfStatisticalProcessing::Missing ||
            window.typeOfTimeIncrement != TypeOfTimeIncrement::Missing ||
            window.timeRange != ProductTimeDuration{tables::TimeUnit::Second, 0} ||
            window.timeIncrement != ProductTimeDuration{tables::TimeUnit::Second, 0} ||
            spec.incrementKind() != TimeIncrementKind::NoIncrement) {
            fail("instant canonical placeholder invariant is violated");
        }
    } else {
        eckit::DateTime expectedStart;
        try {
            expectedStart = subtractDuration(spec.windowEndDateTime(), spec[0].timeRange);
        } catch (const std::exception& e) {
            fail(std::string("cannot recompute outer support start: ") + e.what());
            return;
        }
        if (expectedStart != spec.windowStartDateTime()) {
            fail("absolute support interval is not tied to the outermost canonical range");
        }

        for (const auto& window : spec) {
            if (window.typeOfStatisticalProcessing ==
                tables::TypeOfStatisticalProcessing::Missing) {
                fail("real canonical window has Missing processing type");
            }
        }
    }

    if (spec.kind() == ProductTimeSpecKind::FromStartSingleLoop) {
        if (spec.windowStartDateTime() != spec.referenceDateTime()) {
            fail("from-start support does not begin at referenceDateTime");
        }
        if (resolver_detail::resolvedStep(input) == 0 &&
            !spec.options().allowZeroLengthFsWindow) {
            fail("zero-length from-start object lacks policy evidence");
        }
    }

    if (spec.incrementKind() == TimeIncrementKind::AifsPureMissingIncrement) {
        if (input.marsClass() != "ml" || realStatisticalWindowCount(spec) != 1 ||
            spec[0].typeOfTimeIncrement != TypeOfTimeIncrement::Missing ||
            spec[0].timeIncrement != ProductTimeDuration{tables::TimeUnit::Second, 0}) {
            fail("AIFS-pure missing-increment sentinel invariant is violated");
        }
    }

    if (realStatisticalWindowCount(spec) !=
        realStatisticalWindowCount(classification.shapeType,
                                   input.stattypeBlocks().size())) {
        fail("real statistical window count is inconsistent");
    }
}

}  // namespace metkit::mars2grib::product_time_spec
