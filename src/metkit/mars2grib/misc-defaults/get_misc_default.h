/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <algorithm>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::misc_defaults {

namespace detail {

template <class Cntx_t>
inline std::optional<long> integral(const eckit::LocalConfiguration& mars, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const std::string name{key};
    std::optional<long> result;
    if (mars.has(name) && mars.isIntegral(name)) {
        result = mars.getLong(name);
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
inline std::optional<std::string> string(const eckit::LocalConfiguration& mars, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const std::string name{key};
    std::optional<std::string> result;
    if (mars.has(name) && mars.isString(name)) {
        result = mars.getString(name);
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace detail

/// Return the value used to continue a requirements-discovery run when a
/// mandatory misc key is read. A missing result means that no default policy
/// has been defined yet for the key and type.
template <typename T, class Cntx_t>
std::optional<T> get_misc_default(const eckit::LocalConfiguration& mars, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    std::optional<T> result;

    if constexpr (std::is_same_v<T, bool>) {
        if (key == "bitmapPresent") {
            result = false;
        }
    }
    else if constexpr (std::is_same_v<T, long>) {
        if (key == "numberOfForecastsInEnsemble") result = 51L;
        else if (key == "subCentre") result = 0L;
        else if (key == "scaleFactorOfWaveDirections") result = 2L;
        else if (key == "scaleFactorOfWaveFrequencies") result = 6L;
        else if (key == "pvSize") result = 137L;
        else if (key == "shapeOfTheEarth") result = 6L;
        else if (key == "numberOfFrequencies") result = 54L;
        else if (key == "subSetTruncation") {
            if (const auto truncation =
                    detail::integral(mars, "truncation", utils::profiling::callSite(cntx, Here()))) {
                result = *truncation >= 213L ? 20L : std::min(10L, *truncation);
            }
        }
        else if (key == "typeOfProcessedData") {
            const auto marsClass = detail::string(mars, "class", utils::profiling::callSite(cntx, Here()));
            const auto marsType  = detail::string(mars, "type", utils::profiling::callSite(cntx, Here()));
            if (marsClass && *marsClass == "ai") result = 10L;
            else if (marsType && (*marsType == "an" || *marsType == "me" || *marsType == "4i")) result = 0L;
            else if (marsType && *marsType == "ssd") result = 1L;
            else if (marsType && (*marsType == "fc" || *marsType == "cf")) result = 3L;
            else if (marsType && *marsType == "pf") result = 4L;
            else if (marsType && *marsType == "gsd") result = 6L;
            else if (marsType) result = 255L;
        }
        else if (key == "typeOfEnsembleForecast") {
            const auto marsType = detail::string(mars, "type", utils::profiling::callSite(cntx, Here()));
            if (marsType && (*marsType == "cf" || *marsType == "fc")) result = 5L;
            else if (marsType && *marsType == "pf") result = 6L;
        }
    }

    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::misc_defaults
