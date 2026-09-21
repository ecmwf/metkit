/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file Registry.h
/// @brief Help-only mappings between misc and GRIB keywords.
///
/// Runtime deductions do not dispatch through this registry. It is the
/// authoritative index used after a recording run to assemble user help.
///

#pragma once

#include <array>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/mars2grib/backend/misc-defaults/Types.h"

namespace metkit::mars2grib::backend::misc_defaults {

enum class MiscInputRole : std::uint8_t {
    Alternative,
    Modifier
};

using DataTypesFunction = std::vector<MiscDataType> (*)(std::string_view);
using PolicyFunction = MiscPolicy (*)(std::string_view);
using DescriptionFunction = std::string (*)(std::string_view);

struct MiscRegistration {
    std::string_view miscKey;
    std::string_view gribKey;
    std::string_view alternative;
    MiscInputRole role;
    DataTypesFunction dataTypes;
    PolicyFunction policy;
    DescriptionFunction description;
};

namespace detail {

inline std::vector<MiscDataType> integer_type(std::string_view) { return {MiscDataType::Integer}; }
inline std::vector<MiscDataType> real_type(std::string_view) { return {MiscDataType::Real}; }
inline std::vector<MiscDataType> real_array_type(std::string_view) { return {MiscDataType::RealArray}; }
inline std::vector<MiscDataType> integer_or_string_type(std::string_view) {
    return {MiscDataType::Integer, MiscDataType::String};
}
inline MiscPolicy mandatory(std::string_view) { return MiscPolicy::Mandatory; }
inline MiscPolicy optional(std::string_view) { return MiscPolicy::Optional; }
inline MiscPolicy override_policy(std::string_view) { return MiscPolicy::OptionalOverride; }
inline std::string direct(std::string_view key) {
    return "Provide `" + std::string(key) + "` directly.";
}
inline std::string pv_description(std::string_view key) {
    return key == "pv" ? "Provide the PV coefficients as an array of real values."
                       : "Provide a supported PV table size; 137 is used when neither PV representation is supplied.";
}
inline std::string wave_direction_description(std::string_view key) {
    if (key == "waveDirections") return "Provide wave directions as an array of radians.";
    if (key == "numberOfWaveDirections") return "Reconstruct uniformly spaced wave directions from this count.";
    return "Optional decimal scale factor applied to wave directions; defaults to 2.";
}
inline std::string wave_frequency_description(std::string_view key) {
    if (key == "waveFrequencies") return "Provide wave frequencies directly as an array in hertz.";
    if (key == "scaleFactorOfWaveFrequencies") return "Optional decimal scale factor; defaults to 6.";
    return "Member of the four-key wave-frequency reconstruction alternative.";
}
inline std::string bits_per_value_description(std::string_view) {
    return "Packing precision. Its default depends on the SH/gridded representation family and, for gridded fields, parameter, level type, and compression options.";
}
inline std::string time_increment_description(std::string_view) {
    return "Positive time increment in seconds, accepted as an integer or an integer string.";
}

}  // namespace detail

inline constexpr std::array<MiscRegistration, 33> misc_registry{{
    {"numberOfComponents", "numberOfComponents", "direct", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::direct},
    {"numberOfFourierCoefficients", "numberOfFourierCoefficients", "direct", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::direct},
    {"satelliteSeries", "satelliteSeries", "direct", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::direct},
    {"modelErrorType", "modelErrorType", "direct", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::direct},
    {"tablesVersion", "tablesVersion", "direct", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::direct},
    {"scaledValueOfCentralWaveNumber", "scaledValueOfCentralWaveNumber", "direct", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::direct},
    {"scaleFactorOfCentralWaveNumber", "scaleFactorOfCentralWaveNumber", "direct", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::direct},
    {"generatingProcessIdentifier", "generatingProcessIdentifier", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"numberOfForecastsInEnsemble", "numberOfForecastsInEnsemble", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"subCentre", "subCentre", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"numberOfFrequencies", "numberOfFrequencies", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"shapeOfTheEarth", "shapeOfTheEarth", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"derivedForecast", "derivedForecast", "override", MiscInputRole::Alternative, detail::integer_type, detail::override_policy, detail::direct},
    {"typeOfEnsembleForecast", "typeOfEnsembleForecast", "override", MiscInputRole::Alternative, detail::integer_type, detail::override_policy, detail::direct},
    {"typeOfProcessedData", "typeOfProcessedData", "override", MiscInputRole::Alternative, detail::integer_or_string_type, detail::override_policy, detail::direct},
    {"bitsPerValue", "bitsPerValue", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::bits_per_value_description},
    {"subSetTruncation", "subSetJ", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"timeIncrementInSeconds", "timeIncrement", "direct", MiscInputRole::Alternative, detail::integer_or_string_type, detail::optional, detail::time_increment_description},
    {"totalNumberOfIterations", "totalNumberOfIterations", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"lengthOfTimeWindow", "lengthOf4DvarWindow", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"pv", "pv", "values", MiscInputRole::Alternative, detail::real_array_type, detail::optional, detail::pv_description},
    {"pvSize", "pv", "size", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::pv_description},
    {"waveDirections", "scaledValuesOfWaveDirections", "values", MiscInputRole::Alternative, detail::real_array_type, detail::mandatory, detail::wave_direction_description},
    {"numberOfWaveDirections", "scaledValuesOfWaveDirections", "count", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::wave_direction_description},
    {"scaleFactorOfWaveDirections", "scaledValuesOfWaveDirections", "scale", MiscInputRole::Modifier, detail::integer_type, detail::optional, detail::wave_direction_description},
    {"waveFrequencies", "scaledValuesOfWaveFrequencies", "values", MiscInputRole::Alternative, detail::real_array_type, detail::mandatory, detail::wave_frequency_description},
    {"numberOfWaveFrequencies", "scaledValuesOfWaveFrequencies", "reconstruction", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::wave_frequency_description},
    {"indexOfReferenceWaveFrequency", "scaledValuesOfWaveFrequencies", "reconstruction", MiscInputRole::Alternative, detail::integer_type, detail::mandatory, detail::wave_frequency_description},
    {"referenceWaveFrequency", "scaledValuesOfWaveFrequencies", "reconstruction", MiscInputRole::Alternative, detail::real_type, detail::mandatory, detail::wave_frequency_description},
    {"waveFrequencySpacingRatio", "scaledValuesOfWaveFrequencies", "reconstruction", MiscInputRole::Alternative, detail::real_type, detail::mandatory, detail::wave_frequency_description},
    {"scaleFactorOfWaveFrequencies", "scaledValuesOfWaveFrequencies", "scale", MiscInputRole::Modifier, detail::integer_type, detail::optional, detail::wave_frequency_description},
    {"iTmin", "scaledValueOfLowerWavePeriodLimit", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct},
    {"iTmax", "scaledValueOfUpperWavePeriodLimit", "direct", MiscInputRole::Alternative, detail::integer_type, detail::optional, detail::direct}
}};

inline const MiscRegistration* registration_for_misc_key(std::string_view key) noexcept {
    for (const auto& entry : misc_registry) if (entry.miscKey == key) return &entry;
    return nullptr;
}

inline std::vector<std::string_view> misc_keys_for_grib_key(std::string_view key) {
    std::vector<std::string_view> result;
    for (const auto& entry : misc_registry) if (entry.gribKey == key) result.push_back(entry.miscKey);
    return result;
}

inline std::vector<std::string_view> grib_keys_for_misc_keys(const std::vector<std::string>& keys) {
    std::vector<std::string_view> result;
    for (const auto& key : keys) {
        const auto* entry = registration_for_misc_key(key);
        if (entry == nullptr) continue;
        bool seen = false;
        for (const auto value : result) seen = seen || value == entry->gribKey;
        if (!seen) result.push_back(entry->gribKey);
    }
    return result;
}

}  // namespace metkit::mars2grib::backend::misc_defaults
