// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

///
/// @file brightnessTemperatureMatcher.h
/// @brief Entry-level matcher for the GRIB `brightnessTemperature` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// decide whether the **brightnessTemperature concept** is active for a MARS request.
///
/// The matcher follows the standard mars2grib matching contract:
/// - return a local concept variant index when the concept is active,
/// - return `compile_time_registry_engine::MISSING` when it is not active.
///
/// @note
/// The namespace name `concepts_` is intentionally used instead of
/// `concepts` to avoid conflicts with the C++20 `concept` language
/// feature.
///
/// @ingroup mars2grib_backend_concepts
///

#pragma once

// System includes
#include <cstddef>
#include <string>

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/concepts/brightness-temperature/brightnessTemperatureEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::concepts_ {

/// @brief Entry-level matcher for the `brightnessTemperature` concept.
///
/// The concept is activated for the brightness-temperature ensemble mean:
///
/// - `param == 194`
/// - `stream == "elda"`
/// - presence of the MARS key `channel`
///
/// It owns the brightness-temperature Local Use Section metadata (local
/// definition 37), which is where the channel of the ensemble mean is encoded.
///
/// A brightness temperature of `stream == "oper"` is a satellite product: its
/// channel is encoded in Section 4 by the `satellite` concept (templates
/// 4.204/4.205), so this concept does not apply. Such a request must carry the
/// satellite identification (`ident` and `instrument`); without it the channel
/// could not be encoded anywhere and the request is rejected.
///
/// @tparam MarsDict_t Type of the MARS dictionary
/// @tparam OptDict_t Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt Options dictionary
///
/// @return Index of the selected concept variant, or
/// `compile_time_registry_engine::MISSING` if the concept does not apply
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If the request is identified as a brightness-temperature request but the
/// mandatory `channel` key is missing, or if a `stream=oper` request lacks the
/// satellite identification.
template <class MarsDict_t, class OptDict_t>
std::size_t brightnessTemperatureMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;

    // Concept does not apply unless "param" is present and equals 194
    if (!has(mars, "param") || get_or_throw<long>(mars, "param") != 194) {
        return compile_time_registry_engine::MISSING;
    }

    // Concept does not apply unless the stream is explicitly supported
    if (!has(mars, "stream")) {
        return compile_time_registry_engine::MISSING;
    }

    const auto& stream = get_or_throw<std::string>(mars, "stream");

    if (stream != "oper" && stream != "elda") {
        return compile_time_registry_engine::MISSING;
    }

    // At this point the request is a brightness-temperature request:
    // "channel" is mandatory
    if (!has(mars, "channel")) {
        throw utils::exceptions::Mars2GribMatcherException(
            "brightnessTemperature concept requires MARS key \"channel\" "
            "when param=194 and stream is either \"oper\" or \"elda\"",
            Here());
    }

    if (stream == "elda") {
        return static_cast<std::size_t>(BrightnessTemperatureType::EnsembleMean);
    }

    // stream=oper: a satellite product, the channel goes to Section 4 with the satellite identification
    if (!has(mars, "ident") || !has(mars, "instrument")) {
        throw utils::exceptions::Mars2GribMatcherException(
            "brightness temperature (param=194, stream=oper) requires the MARS keys \"ident\" and \"instrument\": "
            "the channel is encoded with the satellite identification in Section 4",
            Here());
    }

    return compile_time_registry_engine::MISSING;
}

}  // namespace metkit::mars2grib::backend::concepts_