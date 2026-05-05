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
/// @file waveMatcher.h
/// @brief Entry-level matcher for the GRIB `wave` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// identify wave-period and wave-spectra products.
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
#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/backend/concepts/wave/waveEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Match the `wave` concept variant.
///
/// Wave-period parameters resolve to `WaveType::Period`. Wave spectra resolve
/// to `WaveType::Spectra` and require both `frequency` and `direction` keys.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local wave variant index, or
/// `compile_time_registry_engine::MISSING` when the concept is inactive.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If wave spectra metadata is incomplete, the required parameter metadata
/// cannot be read, or lower-level matcher evaluation fails. Lower-level
/// exceptions are preserved through `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t waveMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;
        using metkit::mars2grib::utils::exceptions::Mars2GribMatcherException;

        const auto param = get_or_throw<long>(mars, "param");

        if (matchAny(param, range(140114, 140120))) {
            return static_cast<std::size_t>(WaveType::Period);
        }

        if (matchAny(param, 140251)) {
            if (!has(mars, "frequency")) {
                throw Mars2GribMatcherException("Missing required mars keyword `frequency` for wave spectra", Here());
            }
            if (!has(mars, "direction")) {
                throw Mars2GribMatcherException("Missing required mars keyword `direction` for wave spectra", Here());
            }
            return static_cast<std::size_t>(WaveType::Spectra);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException("Unable to match `wave` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
