// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file satelliteMatcher.h
/// @brief Entry-level matcher for the GRIB `satellite` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether satellite product metadata is active for a MARS request.
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
#include "metkit/mars2grib/backend/concepts/satellite/satelliteEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

///
/// @brief Whether the MARS request describes a satellite product.
///
/// A satellite product carries the full satellite identification: `channel`, `ident` and `instrument`.
///
/// The brightness temperature of the ensemble data assimilation (`param=194`, `stream=elda`) is not a
/// satellite product, whatever other keys the request carries: its `channel` is an index over instrument
/// and channel, not a real channel. It is handled by the `brightnessTemperature` concept and keeps the
/// channel in the local section (local definition 37).
///
/// This is the single definition of the condition: the `satellite` matcher and the `level` matcher (which
/// skips satellite products) both use it.
///
template <class MarsDict_t>
bool isSatelliteProduct(const MarsDict_t& mars) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    if (has(mars, "param") && get_or_throw<long>(mars, "param") == 194 && has(mars, "stream") &&
        get_or_throw<std::string>(mars, "stream") == "elda") {
        return false;
    }

    return has(mars, "channel") && has(mars, "ident") && has(mars, "instrument");
}

///
/// @brief Match the `satellite` concept variant.
///
/// The concept is active as `SatelliteType::Default` for a satellite product,
/// see @ref isSatelliteProduct.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local variant index for `SatelliteType::Default`, or
/// `compile_time_registry_engine::MISSING` when the concept is inactive.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If matcher evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t satelliteMatcher(const MarsDict_t& mars, const OptDict_t& opt) {

    try {

        if (isSatelliteProduct(mars)) {
            return static_cast<std::size_t>(SatelliteType::Default);
        }

        return compile_time_registry_engine::MISSING;
    }
    catch (...) {
        std::throw_with_nested(
            utils::exceptions::Mars2GribMatcherException("Unable to match `satellite` concept", Here()));
    }
}

}  // namespace metkit::mars2grib::backend::concepts_
