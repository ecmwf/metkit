/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file pointInTimeMatcher.h
/// @brief Entry-level matcher for the GRIB `pointInTime` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// identify products that use point-in-time forecast semantics.
///
/// The matcher follows the standard mars2grib matching contract:
/// - return a local concept variant index when the concept is active,
/// - return `compile_time_registry_engine::MISSING` when it is not active,
/// - wrap runtime failures as nested `Mars2GribMatcherException` instances.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System include
#include <cstddef>
#include <exception>

// Utils
#include "metkit/mars2grib/backend/concepts/point-in-time/pointInTimeEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `pointInTime` concept variant.
///
/// The matcher classifies known non-statistical, wave, satellite, and chemical
/// parameters as `PointInTimeType::Default`.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local variant index for `PointInTimeType::Default`, or
/// `compile_time_registry_engine::MISSING` when the concept is inactive.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If the required parameter metadata cannot be read or lower-level matcher
/// evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t pointInTimeMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {

        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;
        using metkit::mars2grib::utils::dict_traits::get_or_throw;

        const auto param = get_or_throw<long>(mars, "param");
        if (matchAny(param, range(1, 3), 10, range(15, 18), range(21, 23), range(26, 43), 53, 54, 59, 60, 66, 67,
                     range(74, 79), range(129, 139), 141, 148, 151, 152, range(155, 157), range(159, 168), 170,
                     range(172, 174), 183, range(186, 188), 198, 203, 206, 207, range(229, 232), range(234, 236), 238,
                     range(243, 248), 3020, 3031, 3067, range(3073, 3075), 129172, range(131074, 131077),
                     range(140098, 140105), 140112, 140113, range(140121, 140129), range(140131, 140134),
                     range(140207, 140209), 140211, 140212, range(140214, 140239), range(140244, 140249),
                     range(140252, 140254), 160198, range(162059, 162063), 162071, 162072, 162093, 174096, 200199,
                     range(210186, 210191), range(210198, 210202), range(210260, 210264), range(213101, 213160),
                     range(228001, 228003), range(228007, 228020), 228023, 228024, 228029, 228032, 228037, 228038,
                     range(228044, 228048), 228050, 228052, range(228088, 228090), 228131, 228132, 228141, 228164,
                     range(228217, 228221), range(228231, 228237), 229001, 229007, 260004, 260005, 260015, 260038,
                     260048, 260109, 260121, 260123, 260132, 260199, 260242, 260255, 260260, 260289, 260290, 260292,
                     260293, 260360, 260509, 260688, 261001, 261002, range(261014, 261016), 261018, 261023,
                     range(262000, 262009), 262011, 262014, 262015, 262017, 262018, 262023, 262024,
                     range(262100, 262106), range(262108, 262112), range(262113, 262116), range(262118, 262125), 262130,
                     range(262139, 262141), 262143, 262144, range(262146, 262149), range(262500, 262502),
                     range(262505, 262507), 262900, 262906, 262907)) {
            return static_cast<std::size_t>(PointInTimeType::Default);
        }

        // Wave products
        if (matchAny(param, range(140114, 140120), 140251)) {
            return static_cast<std::size_t>(PointInTimeType::Default);
        }

        // Satellite products
        if (matchAny(param, 194, range(260510, 260512))) {
            return static_cast<std::size_t>(PointInTimeType::Default);
        }

        // Chemical products
        if (matchAny(param, range(228083, 228085))) {
            return static_cast<std::size_t>(PointInTimeType::Default);
        }

        // ECMWF covariance / analysis-uncertainty paramIds (254001..254017).
        // These are point-in-time products living on the abstractLevel
        // (typeOfFirstFixedSurface=254) and are used with MARS type=est
        // (individual ensemble member, PDT=1) as well as with non-ensemble
        // analyses (PDT=0). Without this mapping, PointInTimeConcept is left
        // inactive and Section 4 recipe selection fails with "No matching recipe".
        if (matchAny(param, range(254001, 254017))) {
            return static_cast<std::size_t>(PointInTimeType::Default);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `pointInTime` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
