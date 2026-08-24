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
/// @file ensembleMatcher.h
/// @brief Entry-level matcher for the GRIB `ensemble` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether ensemble-member metadata is active for a MARS request.
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
#include "metkit/mars2grib/backend/concepts/ensemble/ensembleEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `ensemble` concept variant.
///
/// The concept is active as `EnsembleType::Individual` when the MARS request
/// contains `number`.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local variant index for `EnsembleType::Individual`, or
/// `compile_time_registry_engine::MISSING` when the concept is inactive.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t ensembleMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    // Skip model-error products: in that case "number" identifies the
    // model-error realization, not an ensemble member.
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;


        long param = get_or_throw<long>(mars, "param");
        if (has(mars, "number") &&
            !(has(mars, "type") &&
              (get_or_throw<std::string>(mars, "type") == "me" || get_or_throw<std::string>(mars, "type") == "eme" ||
               get_or_throw<std::string>(mars, "type") == "efi" || get_or_throw<std::string>(mars, "type") == "efic" ||
               get_or_throw<std::string>(mars, "type") == "sot"))) {
            return static_cast<std::size_t>(EnsembleType::Individual);
        }
        else if (has(mars, "type") && (get_or_throw<std::string>(mars, "type") == "ep")) {
            if (matchAny(param, 131060, 131061, 131062, 131063, 131064, 131065, 131066, 131067, 131068, 131069, 131070,
                         131071, 131072, 131073, range(131074, 131077), 131085, 131089, 131090, 131091, 131098, 131099,
                         131100)) {  // }, 133093, 133094, 133095, 133096, 133097, 133098)) {
                return static_cast<std::size_t>(EnsembleType::ProbabilityLargeEnsemble);
            }
            else {
                return compile_time_registry_engine::MISSING;
            }
        }
        else {
            return compile_time_registry_engine::MISSING;
        }
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `ensemble` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
