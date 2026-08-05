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
/// @file longrangeMatcher.h
/// @brief Entry-level matcher for the GRIB `longrange` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether long-range forecast system metadata is active for a request.
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
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/concepts/longrange/longrangeEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {


///
/// @brief Match the `longrange` concept variant.
///
/// The concept is active when both `method` and
/// `system` are present in the MARS request; When stream is equal to "" or ""
/// then the concept is active with the variant LongrangeType::SeasonalForecastMonthlyMean,
/// otherwise the concept is active with the variant LongrangeType::SeasonalForecast.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local variant index for `LongrangeType::SeasonalForecast`, or
/// `compile_time_registry_engine::MISSING` when the concept is inactive.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t longrangeMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        const auto marsStream = get_or_throw<std::string>(mars, "stream");
        const auto marsClass = get_or_throw<std::string>(mars, "class");

        auto isSeasonal = [](const std::string& klass, const std::string& stream) {
            return (klass == "od" || klass == "rd" || klass == "c3") && (stream == "sfmd" || stream == "shmd");
        };

        if (has(mars, "method") && has(mars, "system")) {

            /// @todo review this logic
            if (has(mars, "fcmonth")){
                if (isSeasonal(marsClass, marsStream)) {
                    return static_cast<size_t>(LongrangeType::SeasonalForecastMonthlyMean);
                }
                else {
                    std::ostringstream os;
                    os << "MARS request has `fcmonth` but is not seasonal: class=" << marsClass
                       << ", stream=" << marsStream;
                    throw eckit::SeriousBug(os.str(), Here());
                }
            }
            else {
                return static_cast<size_t>(LongrangeType::SeasonalForecast);
            }
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `longrange` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
