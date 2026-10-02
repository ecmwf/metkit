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
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `referencePeriod` concept variant.
///
/// The concept is active for products defined in relation to a reference period:
/// EFI/SOT (`efi`, `efic`, `sot`), probabilities of (standardised) anomalies, and
/// anomaly and significance parameters.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local variant index, or `compile_time_registry_engine::MISSING`.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t referencePeriodMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {

        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;
        using metkit::mars2grib::utils::dict_traits::get_or_throw;

        const auto marsType = get_or_throw<std::string>(mars, "type");
        const auto param    = get_or_throw<long>(mars, "param");

        // EFI / SOT against the model climate
        if (marsType == "efi" || marsType == "efic" || marsType == "sot") {
            return static_cast<std::size_t>(ReferencePeriodType::Default);
        }

        // Probabilities of standardised anomalies
        if (matchAny(param, range(133093, 133098))) {
            return static_cast<std::size_t>(ReferencePeriodType::Default);
        }

        // Probabilities of anomalies (see `Probability::Anomaly`)
        if (marsType == "ep" && matchAny(param, range(131001, 131010), range(131020, 131025))) {
            return static_cast<std::size_t>(ReferencePeriodType::Default);
        }

        // 171??? / 173??? -> Anomaly parameters
        // 234???          -> Significance parameters
        if (matchAny(param, range(171000, 171999), range(173000, 173999), range(234000, 234999))) {
            return static_cast<std::size_t>(ReferencePeriodType::Default);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `referencePeriod` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
