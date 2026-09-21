/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Types.h
/// @brief Common value and metadata types for misc-default resolution.

#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

namespace metkit::mars2grib::backend::misc_defaults {

enum class MiscPolicy : std::uint8_t {
    Mandatory,
    Optional,
    OptionalOverride
};

enum class MiscSource : std::uint8_t {
    Explicit,
    Default
};

enum class MiscDataType : std::uint8_t {
    Boolean,
    Integer,
    Real,
    String,
    IntegerArray,
    RealArray,
    StringArray
};

template <class T>
struct ResolvedMisc {
    T value;
    MiscSource source{MiscSource::Default};
    std::string_view representation{};
};

struct MiscInput {
    std::string_view key;
    std::vector<MiscDataType> types;
    std::string description;
};

struct MiscAlternative {
    std::vector<MiscInput> inputs;
    std::string description;
};

struct MiscHelp {
    std::string gribKey;
    MiscPolicy policy{MiscPolicy::Optional};
    std::vector<MiscAlternative> alternatives;
    std::vector<MiscInput> modifiers;
    std::optional<std::string> effectiveDefault;
    std::string description;
};

struct WaveDirectionInput {
    std::vector<double> directions;
    std::optional<long> numberOfDirections;
    long scaleFactor{2};
};

struct WaveFrequencyInput {
    std::vector<double> frequencies;
    std::optional<long> numberOfFrequencies;
    std::optional<long> referenceIndex;
    std::optional<double> referenceFrequency;
    std::optional<double> spacingRatio;
    long scaleFactor{6};
};

struct PvArrayInput {
    std::optional<std::vector<double>> values;
    long size{137};
};

struct TimeIncrement {
    long seconds{0};
};

using IntegerOrString = std::variant<long, std::string>;

}  // namespace metkit::mars2grib::backend::misc_defaults
