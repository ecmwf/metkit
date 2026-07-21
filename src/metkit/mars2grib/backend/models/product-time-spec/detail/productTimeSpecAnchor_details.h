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
/// @file productTimeSpecAnchor_details.h
/// @brief Internal helpers for ProductTimeSpec anchor classification and build.
///
/// This header contains the complete header-only implementation behind the
/// public anchor API declared in `ProductTimeSpecAnchor.h`.
///

#pragma once

#include <string>

#include "eckit/types/DateTime.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecDataTypes.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models::detail {

///
/// @brief Materialize the resolved label datetime from normalized input.
///
/// Direct-source precedence is:
/// - `hindcastDateTime`;
/// - otherwise `dateTime`;
/// - otherwise `yearMonthDateTime`.
///
/// The caller must ensure that at least one direct source exists.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @return Resolved label datetime.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException if no
///         direct anchor source is available.
///
template <class Input_t>
eckit::DateTime buildDirectLabelDateTime_or_throw(const Input_t& input) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (input.hindcastDateTime.has_value()) {
        return *input.hindcastDateTime;
    }
    if (input.dateTime.has_value()) {
        return *input.dateTime;
    }
    if (input.yearMonthDateTime.has_value()) {
        return *input.yearMonthDateTime;
    }

    throw Mars2GribModelException(
        "Unable to build ProductTimeSpec direct label datetime from normalized input",
        input.to_json(),
        Here());

    mars2gribUnreachable();
}

///
/// @brief Build the direct initial-conditions datetime from normalized input.
///
/// This helper materializes the second datetime of the resolved anchor, namely
/// `initialConditionsDateTime`. The function does not perform classification;
/// it applies the anchor inheritance rule that governs how the direct `dateTime`
/// source participates in anchor construction once the label datetime has
/// already been resolved.
///
/// The logic is intentionally simple and fully centralized here:
/// 1. when a normalized direct `dateTime` source is present, that value
///    is the direct initial-conditions datetime and must be returned unchanged;
/// 2. otherwise there is no dedicated `dateTime` source, so the initial-
///    conditions datetime is inherited from the already-resolved
///    `labelDateTime`.
///
/// The caller passes `labelDateTime` explicitly because that value is already a
/// resolved anchor component and is the only fallback permitted by the model at
/// this stage. This keeps the inheritance rule explicit and avoids rebuilding
/// label-source logic here.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] labelDateTime Already resolved label datetime used as the only
///            permitted fallback when no direct `dateTime` source exists.
/// @return Resolved direct-or-inherited initial-conditions datetime.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException may
///         propagate from future normalization or validation extensions that
///         need normalized-input context.
///
template <class Input_t>
eckit::DateTime buildDirectInitialConditionsDateTime_or_throw(const Input_t& input,
                                                              const eckit::DateTime& labelDateTime) {
    if (input.dateTime.has_value()) {
        return *input.dateTime;
    }

    return labelDateTime;
}

///
/// @brief Build the direct reference datetime from normalized input.
///
/// This helper materializes the third datetime of the resolved anchor, namely
/// `referenceDateTime`. As with the initial-conditions helper, the function is
/// not a classification step; it applies the reference-time inheritance rule
/// after the preceding anchor component has already been resolved.
///
/// The logic is:
/// 1. when a normalized direct `yearMonthDateTime` source is present, that
///    value is the direct reference datetime and must be returned unchanged;
/// 2. otherwise there is no dedicated year/month reference anchor, so the
///    reference datetime is inherited from the already-resolved
///    `initialConditionsDateTime`.
///
/// The caller passes `initialConditionsDateTime` explicitly because that value
/// is the only valid fallback for the reference anchor once normalized input has
/// been interpreted according to the ProductTimeSpec anchor model.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @param[in] initialConditionsDateTime Already resolved initial-conditions
///            datetime used as the only permitted fallback when no direct
///            year/month reference source exists.
/// @return Resolved direct-or-inherited reference datetime.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException may
///         propagate from future normalization or validation extensions that
///         need normalized-input context.
///
template <class Input_t>
eckit::DateTime buildDirectReferenceDateTime_or_throw(const Input_t& input,
                                                      const eckit::DateTime& initialConditionsDateTime) {
    if (input.yearMonthDateTime.has_value()) {
        return *input.yearMonthDateTime;
    }

    return initialConditionsDateTime;
}

///
/// @brief Validate the final ordering invariant of a resolved anchor.
///
/// Every successfully built anchor must satisfy
/// `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
///
/// @tparam Input_t Normalized ProductTimeSpec model-input type.
/// @param[in] labelDateTime Resolved label datetime.
/// @param[in] initialConditionsDateTime Resolved initial-conditions datetime.
/// @param[in] referenceDateTime Resolved reference datetime.
/// @param[in] input Normalized ProductTimeSpec input snapshot.
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribModelException when
///         the required ordering is violated.
///
template <class Input_t>
void checkProductTimeSpecAnchorOrdering_or_throw(const eckit::DateTime& labelDateTime,
                                                 const eckit::DateTime& initialConditionsDateTime,
                                                 const eckit::DateTime& referenceDateTime,
                                                 const Input_t& input) {

    using metkit::mars2grib::utils::exceptions::Mars2GribModelException;

    if (labelDateTime > initialConditionsDateTime ||
        initialConditionsDateTime > referenceDateTime) {

        throw Mars2GribModelException(
            "ProductTimeSpec anchor ordering invariant `labelDateTime <= initialConditionsDateTime <= referenceDateTime` is violated",
            input.to_json(),
            Here());
    }
}

}  // namespace metkit::mars2grib::backend::models::detail
