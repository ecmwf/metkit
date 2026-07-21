/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#pragma once

#include <cstddef>

#include "eckit/types/DateTime.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchorClassification.h"
#include "metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecAnchor_details.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::models {

///
/// @brief Fully resolved ordered ProductTimeSpec anchor artifact.
///
/// The artifact stores the three anchor datetimes used by later ProductTimeSpec
/// stages together with the direct-source regime that produced them.
///
/// Valid resolved anchors satisfy:
/// `labelDateTime <= initialConditionsDateTime <= referenceDateTime`.
///
struct ProductTimeSpecAnchor {
    eckit::DateTime labelDateTime{};
    eckit::DateTime initialConditionsDateTime{};
    eckit::DateTime referenceDateTime{};
    TimeAnchorKind anchorType{TimeAnchorKind::LabelOnly};
};

///
/// @brief Absolute temporal support interval of one resolved ProductTimeSpec.
///
/// The domain artifact stores the start and end datetimes of the product's
/// resolved support. The start and end are absolute placements, not relative
/// durations.
///
struct ProductTimeSpecDomain {

    /// @brief Absolute start datetime of the resolved product support.
    eckit::DateTime domainStartDateTime{};

    /// @brief Absolute end datetime of the resolved product support.
    eckit::DateTime domainEndDateTime{};

};

///
/// @brief One canonical ProductTimeSpec statistical window.
///
/// Each canonical window stores the statistical processing applied over that
/// window, its range, and the increment associated with samples contributing to
/// it.
///
struct ProductTimeSpecWindow {
    /// @brief Statistical processing performed over this canonical window.
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing{
        tables::TypeOfStatisticalProcessing::Missing};

    /// @brief Length of the canonical statistical window.
    deductions::TimeDuration timeRange{};

    /// @brief Increment associated with samples contributing to the window.
    deductions::TimeDuration timeIncrement{};
};

///
/// @brief Ordered canonical ProductTimeSpec window sequence.
///
/// Windows are stored in outermost-to-innermost order.
///
struct ProductTimeSpecWindows {
    /// @brief Canonical windows in outermost-to-innermost order.
    std::vector<ProductTimeSpecWindow> values{};
};

}  // namespace metkit::mars2grib::backend::models
