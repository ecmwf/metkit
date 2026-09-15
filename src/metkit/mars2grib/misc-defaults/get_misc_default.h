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
#include <vector>

#include "eckit/config/LocalConfiguration.h"

namespace metkit::mars2grib::misc_defaults {

namespace detail {

inline std::optional<long> integral(const eckit::LocalConfiguration& mars, std::string_view key) {
    const std::string name{key};
    if (mars.has(name) && mars.isIntegral(name)) {
        return mars.getLong(name);
    }
    return std::nullopt;
}

inline std::optional<std::string> string(const eckit::LocalConfiguration& mars, std::string_view key) {
    const std::string name{key};
    if (mars.has(name) && mars.isString(name)) {
        return mars.getString(name);
    }
    return std::nullopt;
}

}  // namespace detail

/// Return the value used to continue a requirements-discovery run when a
/// mandatory misc key is read. A missing result means that no default policy
/// has been defined yet for the key and type.
template <typename T>
std::optional<T> get_misc_default(const eckit::LocalConfiguration&, std::string_view) {
    return std::nullopt;
}

template <>
inline std::optional<bool> get_misc_default<bool>(const eckit::LocalConfiguration&, std::string_view key) {
    if (key == "bitmapPresent") {
        return false;
    }

    return std::nullopt;
}

template <>
inline std::optional<long> get_misc_default<long>(const eckit::LocalConfiguration& mars, std::string_view key) {
    if (key == "numberOfForecastsInEnsemble") {
        return 51L;
    }
    if (key == "subCentre") {
        return 0L;
    }
    if (key == "scaleFactorOfWaveDirections") {
        return 2L;
    }
    if (key == "scaleFactorOfWaveFrequencies") {
        return 6L;
    }
    if (key == "pvSize") {
        return 137L;
    }
    if (key == "shapeOfTheEarth") {
        return 6L;
    }
    if (key == "numberOfFrequencies") {
        return 54L;
    }
    if (key == "subSetTruncation") {
        if (const auto truncation = detail::integral(mars, "truncation")) {
            return *truncation >= 213L ? 20L : std::min(10L, *truncation);
        }
        return std::nullopt;
    }
    if (key == "typeOfProcessedData") {
        const auto marsClass = detail::string(mars, "class");
        const auto marsType  = detail::string(mars, "type");
        if (marsClass && *marsClass == "ai") {
            return 10L;
        }
        if (!marsType) {
            return std::nullopt;
        }
        if (*marsType == "an" || *marsType == "me" || *marsType == "4i") {
            return 0L;
        }
        if (*marsType == "ssd") {
            return 1L;
        }
        if (*marsType == "fc" || *marsType == "cf") {
            return 3L;
        }
        if (*marsType == "pf") {
            return 4L;
        }
        if (*marsType == "gsd") {
            return 6L;
        }
        return 255L;
    }
    if (key == "typeOfEnsembleForecast") {
        const auto marsType = detail::string(mars, "type");
        if (!marsType) {
            return std::nullopt;
        }
        if (*marsType == "cf" || *marsType == "fc") {
            return 5L;
        }
        if (*marsType == "pf") {
            return 6L;
        }
        return std::nullopt;
    }
    if (key == "derivedForecast") {
        // TODO: Populate the complete MARS type to derived-forecast mapping.
        return std::nullopt;
    }
    if (key == "bitsPerValue") {
        // TODO: This default also depends on packing options and the active representation.
        return std::nullopt;
    }
    if (key == "satelliteSeries" || key == "scaleFactorOfCentralWaveNumber" ||
        key == "scaledValueOfCentralWaveNumber" || key == "modelErrorType" || key == "numberOfComponents" ||
        key == "numberOfFourierCoefficients" || key == "tablesVersion" || key == "numberOfWaveDirections" ||
        key == "numberOfWaveFrequencies" || key == "indexOfReferenceWaveFrequency" || key == "iTmin" ||
        key == "iTmax" || key == "timeIncrementInSeconds" || key == "generatingProcessIdentifier" ||
        key == "lengthOfTimeWindow" || key == "totalNumberOfIterations") {
        // TODO: Populate mandatory defaults, where a real default policy exists.
        return std::nullopt;
    }

    return std::nullopt;
}

template <>
inline std::optional<double> get_misc_default<double>(const eckit::LocalConfiguration&, std::string_view key) {
    if (key == "missingValue" || key == "scaleValuesBy" || key == "offsetValuesBy" ||
        key == "referenceWaveFrequency" || key == "waveFrequencySpacingRatio") {
        // TODO: Populate defaults that are meaningful independently of the encoded values.
        return std::nullopt;
    }

    return std::nullopt;
}

template <>
inline std::optional<std::string> get_misc_default<std::string>(const eckit::LocalConfiguration&,
                                                                 std::string_view key) {
    if (key == "typeOfProcessedData" || key == "timeIncrementInSeconds") {
        // TODO: Populate string defaults where this representation is preferred.
        return std::nullopt;
    }

    return std::nullopt;
}

template <>
inline std::optional<std::vector<double>> get_misc_default<std::vector<double>>(
    const eckit::LocalConfiguration&, std::string_view key) {
    if (key == "pv" || key == "waveDirections" || key == "waveFrequencies") {
        // TODO: Populate vector defaults where they can be represented safely.
        return std::nullopt;
    }

    return std::nullopt;
}

}  // namespace metkit::mars2grib::misc_defaults
