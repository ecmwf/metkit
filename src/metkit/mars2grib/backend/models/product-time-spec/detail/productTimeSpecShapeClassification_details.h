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
/// @file productTimeSpecShapeClassification_details.h
/// @brief Internal helpers for ProductTimeSpec shape classification.
///

#pragma once

#include <cstddef>
#include <string>

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::detail {

///
/// @brief Test whether one normalized duration is exactly zero.
///
/// Shape classification uses this helper when enforcing the special
/// analysis-step rule. The normalized model input may encode zero either as
/// zero seconds or as another zero-valued unit, so the helper treats any
/// duration with `length == 0` as zero regardless of its unit.
///
/// @param[in] duration Normalized duration value.
/// @return `true` when the duration length is zero.
///
inline bool isZeroProductTimeSpecDuration(const deductions::TimeDuration& duration) {
    return duration.length == 0;
}

///
/// @brief Test whether the normalized resolved `step` is structurally zero.
///
/// Shape classification treats a missing `step` as structurally distinct from a
/// present zero-valued `step`, but the from-start zero-length window rule cares
/// only about whether the resolved explicit value is zero once the presence rule
/// has already been checked elsewhere. This helper therefore inspects the
/// normalized optional `step` and reports zero only when a value is present and
/// its normalized duration length is zero.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return `true` when normalized `step` is present and zero-valued.
///
template <class Input_t>
bool isZeroProductTimeSpecResolvedStep(const Input_t& input) {
    return input.step.has_value() && isZeroProductTimeSpecDuration(*input.step);
}

///
/// @brief Count parsed `stattype` blocks stored in normalized model input.
///
/// The model input preserves source absence through `std::optional`, so a
/// missing `stattype` key is represented by `std::nullopt`, while a present
/// `stattype` is represented by the parsed block sequence. Shape classification
/// depends only on the number of parsed blocks, not on their detailed contents.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return Number of parsed `stattype` blocks: zero when the source is absent.
///
template <class Input_t>
std::size_t countProductTimeSpecShapeStatTypeBlocks(const Input_t& input) {
    return input.stattype.has_value() ? input.stattype->size() : 0;
}

///
/// @brief Return the normalized `timespan` kind used by shape classification.
///
/// The model input preserves source absence as `std::nullopt`, whereas the
/// shape classifier wants one explicit local state in every branch. This helper
/// converts the optional representation into the classifier's four-way source
/// kind by mapping absence to `TimespanKind::Missing` and otherwise forwarding
/// the already-normalized source kind unchanged.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return Explicit `TimespanKind` value for classification.
///
template <class Input_t>
deductions::TimespanKind productTimeSpecShapeTimespanKind(const Input_t& input) {
    return input.timespan.has_value() ? input.timespan->kind : deductions::TimespanKind::Missing;
}

///
/// @brief Enforce the local `type`/`step` consistency rules of shape classification.
///
/// The shape classifier owns two local rules involving the normalized product
/// type and the presence or value of `step`:
/// 1. analysis products (`marsType == "an"`) may carry no explicit `step`, or a
///    zero-valued explicit `step`, but must reject an explicit non-zero step;
/// 2. non-analysis products must carry a normalized `step` value and therefore
///    reject the missing-step state.
///
/// These checks are performed before any `timespan`/`stattype` branching because
/// they constrain the local validity of the shape input regardless of which
/// structural shape is eventually chosen.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if the
///         local `type`/`step` consistency rules are violated.
///
template <class Input_t>
void checkProductTimeSpecShapeStepConsistency_or_throw(const Input_t& input) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (input.marsType == "an") {
        if (input.step.has_value() && !isZeroProductTimeSpecDuration(*input.step)) {
            throw Mars2GribModelException(
                "Analysis product has an explicit non-zero normalized `step`",
                input.to_json(),
                Here());
        }
        return;
    }

    if (!input.step.has_value()) {
        throw Mars2GribModelException(
            "Missing normalized `step` is allowed only for `type=an`",
            input.to_json(),
            Here());
    }
}

///
/// @brief Test whether a `(class, stream)` pair requires fake-double-loop representation.
///
/// This helper isolates the representation-policy lookup used by shape
/// classification for source patterns with `timespan=none` and exactly one
/// parsed `stattype` block, as well as the rejection of the standard
/// single-loop syntax when that compatibility representation is required.
///
/// The current allow-list is:
/// - `e6`: `sttd`, `stte`;
/// - `od`, `rd`, `c3`: `sfmd`, `shmd`;
/// - `gh`, `eh`: `msmm`, `rfsd`.
///
/// @param[in] marsClass Normalized MARS `class` value.
/// @param[in] marsStream Normalized MARS `stream` value.
/// @return `true` when the pair requires fake-double-loop representation.
///
inline bool requiresFakeDoubleLoopRepresentation(const std::string& marsClass,
                                                 const std::string& marsStream) {
    if (marsClass == "e6" && (marsStream == "sttd" || marsStream == "stte")) {
        return true;
    }

    if ((marsClass == "od" || marsClass == "rd" || marsClass == "c3") &&
        (marsStream == "sfmd" || marsStream == "shmd")) {
        return true;
    }

    if ((marsClass == "gh" || marsClass == "eh") &&
        (marsStream == "msmm" || marsStream == "rfsd")) {
        return true;
    }

    return false;
}

///
/// @brief Test whether a source-single-loop product is the reserved fake-single-loop/double-loop case.
///
/// This helper is the dedicated policy hook for the future index-statistics
/// shape whose source syntax contains a duration-valued `timespan`, no
/// `stattype`, and a `(type, class, paramId)` combination that must be lowered
/// as two canonical windows of identical size.
///
/// The identification domain is intentionally not implemented yet. The helper
/// currently returns `false` unconditionally so that the enum value and the
/// public classification table can be introduced without changing observable
/// behavior until the exact policy is finalized.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return `false` for every input in the current placeholder implementation.
///
template <class Input_t>
bool is_FakeSingleLoopDoubleLoop(const Input_t& input) {
    (void)input;
    return false;
}

}  // namespace metkit::mars2grib::backend::models::detail
