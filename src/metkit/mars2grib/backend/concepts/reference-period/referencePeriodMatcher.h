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
/// @file referencePeriodMatcher.h
/// @brief Entry-level matcher for the GRIB `referencePeriod` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether the `referencePeriod` concept is active for a request.
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
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/reference-period/referencePeriodEnum.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/Profiling.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `referencePeriod` concept variant.
///
/// The `referencePeriod` concept is currently inactive and therefore always
/// resolves to `compile_time_registry_engine::MISSING`.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return `compile_time_registry_engine::MISSING`.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::size_t referencePeriodMatcherImpl(const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    try {

        const auto matchAny = [&cntx](auto&&... args) {
            return metkit::mars2grib::util::param_matcher::matchAny(args..., cntx);
        };
        using metkit::mars2grib::utils::dict_traits::get_or_throw;

        const auto marsType = get_or_throw<std::string>(mars, "type", utils::profiling::callSite(cntx, Here()));
        const auto param    = get_or_throw<long>(mars, "param", utils::profiling::callSite(cntx, Here()));

        if (marsType == "efi" || marsType == "efic" || marsType == "sot") {
            const std::size_t result = static_cast<std::size_t>(ReferencePeriodType::Default);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }

        // Check for standardised anomaly parameters
        if (matchAny(param, 133093, 133094, 133095, 133096, 133097, 133098)) {
            const std::size_t result = static_cast<std::size_t>(ReferencePeriodType::Default);
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }


        const std::size_t result = compile_time_registry_engine::MISSING;
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `referencePeriod` concept", Here()));
    }
}

template <class MarsDict_t, class OptDict_t, class Cntx_t>
std::size_t referencePeriodMatcher(const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const std::size_t result = referencePeriodMatcherImpl(mars, opt, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::backend::concepts_
