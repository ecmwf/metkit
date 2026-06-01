/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file brightnessTemperatureEnum.h
/// @brief Definition of the `brightnessTemperature` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB
/// `brightnessTemperature` concept used by the mars2grib backend.
/// It contains:
///
/// - the canonical concept name (`brightnessTemperatureName`)
/// - the enumeration of supported brightness-temperature variants
/// - a compile-time typelist of all variants
/// - a compile-time mapping from variant to string identifier
///
/// This file intentionally contains **no runtime logic** and **no encoding
/// behavior**. Its sole purpose is to provide compile-time metadata used by:
///
/// - the concept registry
/// - compile-time table generation
/// - logging and diagnostics
/// - static validation of concept variants
///
/// @note
/// Brightness temperature is represented as an independent concept because
/// it is orthogonal to the surrounding product-family concept. Depending on
/// the stream, the same parameter may coexist with either the satellite path
/// or the derived-product path.
///
/// @ingroup mars2grib_backend_concepts

#pragma once

// System includes
#include <cstddef>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::backend::concepts_ {

template <auto... Vs>
using ValueList = metkit::mars2grib::backend::compile_time_registry_engine::ValueList<Vs...>;

/// @brief Canonical name of the `brightnessTemperature` concept.
///
/// This identifier is used:
/// - as the logical concept key in the concept registry
/// - for logging and debugging output
/// - to associate variants and capabilities with the concept
///
/// The value must remain stable across releases.
inline constexpr std::string_view brightnessTemperatureName{"brightnessTemperature"};

/// @brief Enumeration of all supported `brightnessTemperature` concept variants.
///
/// The concept currently has a single variant because both supported streams
/// share the same Local Use Section encoding logic.
///
/// The numeric values of the enumerators are **not semantically relevant**;
/// they are required only to:
/// - provide a stable compile-time identifier
/// - allow array indexing and table generation
///
/// @warning
/// Do not reorder existing enumerators, as they are used in compile-time
/// tables and registries.
enum class BrightnessTemperatureType : std::size_t {
    EnsembleMean = 0,
    Default
};

/// @brief Compile-time list of all `brightnessTemperature` concept variants.
///
/// This typelist is used to:
/// - generate concept capability tables at compile time
/// - register all supported variants in the concept registry
/// - enable static iteration over variants without runtime overhead
///
/// @note
/// The order of this list must match the intended iteration order for
/// registry construction and diagnostics.
using BrightnessTemperatureList =
    ValueList<BrightnessTemperatureType::EnsembleMean, BrightnessTemperatureType::Default>;

/// @brief Compile-time mapping from `BrightnessTemperatureType` to human-readable name.
///
/// This function returns the canonical string identifier associated with a
/// given brightness-temperature variant.
///
/// The returned value is used for:
/// - logging and debugging output
/// - error reporting
/// - concept registry diagnostics
///
/// @tparam T Brightness-temperature variant
/// @return String view identifying the variant
///
/// @note
/// The returned string must remain stable across releases, as it may appear
/// in logs, tests, and diagnostic output.
template <BrightnessTemperatureType T>
constexpr std::string_view brightnessTemperatureTypeName();

#define DEF(T, NAME)                                                \
    template <>                                                     \
    constexpr std::string_view brightnessTemperatureTypeName<T>() { \
        return NAME;                                                \
    }

DEF(BrightnessTemperatureType::EnsembleMean, "ensembleMean");
DEF(BrightnessTemperatureType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::concepts_