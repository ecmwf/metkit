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
/// @file levelEnum.h
/// @brief Definition of the `level` concept variants and compile-time metadata.
///
/// This header defines the **static description** of the GRIB `level` concept
/// used by the mars2grib backend. It contains:
///
/// - the canonical concept name (`levelName`)
/// - the exhaustive enumeration of supported level variants (`LevelType`)
/// - a compile-time typelist of all variants (`LevelList`)
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
/// This header is part of the **concept definition layer**.
/// Runtime behavior is implemented separately in the corresponding
/// `level.h` / `levelOp` implementation.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System includes
#include <cstdint>
#include <string_view>

// Core concept includes
#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/utils/generalUtils.h"


namespace metkit::mars2grib::backend::concepts_ {

template <auto... Vals>
using ValueList = metkit::mars2grib::backend::compile_time_registry_engine::ValueList<Vals...>;

///
/// @brief Canonical name of the `level` concept.
///
/// This identifier is used:
/// - as the logical concept key in the concept registry
/// - for logging and debugging output
/// - to associate variants and capabilities with the `level` concept
///
/// The value must remain stable across releases.
///
inline constexpr std::string_view levelName{"level"};


///
/// @brief Enumeration of all supported `level` concept variants.
///
/// Each enumerator represents a distinct GRIB vertical level or layer
/// interpretation as defined by the GRIB2 specification and ECMWF
/// conventions.
///
/// The numeric values of the enumerators are **not semantically relevant**;
/// they are required only to:
/// - provide a stable compile-time identifier
/// - allow array indexing and table generation
///
/// @note
/// This enumeration is intentionally exhaustive and includes both:
/// - concrete GRIB levels (e.g. isobaric, hybrid, heightAboveGround)
/// - abstract or logical levels used internally by the encoder
///
/// @note
/// Model-level coordinates are split into two variants:
/// - `ModelSingleLevel` for fields published as a single 2D layer on the
///   model-level system (no vertical column, no PV array).
/// - `ModelMultipleLevel` for full vertical columns of model-level data,
///   which require allocation and population of the PV array describing
///   the vertical hybrid coordinate transformation.
/// Both variants share the GRIB `typeOfLevel` string `"hybrid"`; they
/// differ only in encoder behaviour (PV allocation).
///
/// @note
/// Three abstract variants exist:
/// - `AbstractSingleLevel` and `AbstractMultipleLevel`: opaque level
///   identifiers without an associated numeric `level` value.
/// - `AbstractLevel`: opaque level identifier that carries a numeric
///   `level` value (encoded via the `level` GRIB key).
///
/// @warning
/// Do not reorder existing enumerators in future changes, as their
/// numeric values are used in compile-time tables and registries.
///
enum class LevelType : std::size_t {
    Surface = 0,
    EntireAtmosphere,
    EntireLake,
    CloudBase,
    Tropopause,
    NominalTop,
    MostUnstableParcel,
    MixedLayerParcel,
    Isothermal,
    IsobaricInPa,
    IsobaricInHpa,
    LowCloudLayer,
    MediumCloudLayer,
    HighCloudLayer,
    MeanSea,
    HeightAboveSea,
    HeightAboveGround,
    ModelSingleLevel,    ///< 2D field on model-level system; no PV array.
    ModelMultipleLevel,  ///< Full vertical column of model-level data; requires PV array.
    Theta,
    PotentialVorticity,
    SnowLayer,
    SoilLayer,
    SeaIceLayer,
    OceanSurface,
    DepthBelowSeaLayer,
    OceanSurfaceToBottom,
    LakeBottom,
    MixingLayer,
    OceanModel,
    OceanModelLayer,
    MixedLayerDepthByDensity,
    MixedLayerDepthByTemperature,
    SnowLayerOverIceOnWater,
    IceTopOnWater,
    IceLayerOnWater,
    EntireMeltPond,
    WaterSurfaceToIsothermalOceanLayer,
    AbstractSingleLevel,
    AbstractLevel,  ///< Opaque level identifier carrying a numeric `level` value.
    AbstractMultipleLevel,
    HeightAboveSeaAt10M,
    HeightAboveSeaAt2M,
    HeightAboveGroundAt10M,
    HeightAboveGroundAt2M,
    Default
};


///
/// @brief Compile-time list of all `level` concept variants.
///
/// This typelist is used to:
/// - generate concept capability tables at compile time
/// - register all supported variants in the concept registry
/// - enable static iteration over variants without runtime overhead
///
/// @note
/// The order of this list must match the intended iteration order
/// for registry construction and diagnostics.
///
using LevelList =
    ValueList<LevelType::Surface, LevelType::EntireAtmosphere, LevelType::EntireLake, LevelType::CloudBase,
              LevelType::Tropopause, LevelType::NominalTop, LevelType::MostUnstableParcel, LevelType::MixedLayerParcel,
              LevelType::Isothermal, LevelType::IsobaricInPa, LevelType::IsobaricInHpa, LevelType::LowCloudLayer,
              LevelType::MediumCloudLayer, LevelType::HighCloudLayer, LevelType::MeanSea, LevelType::HeightAboveSea,
              LevelType::HeightAboveGround, LevelType::ModelSingleLevel, LevelType::ModelMultipleLevel,
              LevelType::Theta, LevelType::PotentialVorticity, LevelType::SnowLayer, LevelType::SoilLayer,
              LevelType::SeaIceLayer, LevelType::OceanSurface, LevelType::DepthBelowSeaLayer,
              LevelType::OceanSurfaceToBottom, LevelType::LakeBottom, LevelType::MixingLayer, LevelType::OceanModel,
              LevelType::OceanModelLayer, LevelType::MixedLayerDepthByDensity, LevelType::MixedLayerDepthByTemperature,
              LevelType::SnowLayerOverIceOnWater, LevelType::IceTopOnWater, LevelType::IceLayerOnWater,
              LevelType::EntireMeltPond, LevelType::WaterSurfaceToIsothermalOceanLayer, LevelType::AbstractSingleLevel,
              LevelType::AbstractLevel, LevelType::AbstractMultipleLevel, LevelType::HeightAboveSeaAt10M,
              LevelType::HeightAboveSeaAt2M, LevelType::HeightAboveGroundAt10M, LevelType::HeightAboveGroundAt2M,
              LevelType::Default>;


///
/// @brief Compile-time mapping from `LevelType` to human-readable name.
///
/// This function returns the canonical string identifier associated
/// with a given level variant.
///
/// The returned value is used for:
/// - logging and debugging output
/// - error reporting
/// - concept registry diagnostics
///
/// @tparam T Level variant
/// @return String view identifying the variant
///
/// @note
/// The returned string must remain stable across releases, as it may
/// appear in logs, tests, and diagnostic output.
///
template <LevelType T>
constexpr std::string_view levelTypeName();

#define DEF(T, NAME)                                \
    template <>                                     \
    constexpr std::string_view levelTypeName<T>() { \
        return NAME;                                \
    }

DEF(LevelType::Surface, "surface");
DEF(LevelType::EntireAtmosphere, "entireAtmosphere");
DEF(LevelType::EntireLake, "entireLake");
DEF(LevelType::CloudBase, "cloudBase");
DEF(LevelType::Tropopause, "tropopause");
DEF(LevelType::NominalTop, "nominalTop");
DEF(LevelType::MostUnstableParcel, "mostUnstableParcel");
DEF(LevelType::MixedLayerParcel, "mixedLayerParcel");
DEF(LevelType::Isothermal, "isothermal");
DEF(LevelType::IsobaricInPa, "isobaricInPa");
DEF(LevelType::IsobaricInHpa, "isobaricInhPa");
DEF(LevelType::LowCloudLayer, "lowCloudLayer");
DEF(LevelType::MediumCloudLayer, "mediumCloudLayer");
DEF(LevelType::HighCloudLayer, "highCloudLayer");
DEF(LevelType::MeanSea, "meanSea");
DEF(LevelType::HeightAboveSea, "heightAboveSea");
DEF(LevelType::HeightAboveGround, "heightAboveGround");
// ModelSingleLevel and ModelMultipleLevel both encode as GRIB
// `typeOfLevel="hybrid"`; they differ only in encoder behaviour
// (PV array allocation, see needPv in levelEncoding.h).
DEF(LevelType::ModelSingleLevel, "hybrid");
DEF(LevelType::ModelMultipleLevel, "hybrid");
DEF(LevelType::Theta, "theta");
DEF(LevelType::PotentialVorticity, "potentialVorticity");
DEF(LevelType::SnowLayer, "snowLayer");
DEF(LevelType::SoilLayer, "soilLayer");
DEF(LevelType::SeaIceLayer, "seaIceLayer");
DEF(LevelType::OceanSurface, "oceanSurface");
DEF(LevelType::DepthBelowSeaLayer, "depthBelowSeaLayer");
DEF(LevelType::OceanSurfaceToBottom, "oceanSurfaceToBottom");
DEF(LevelType::LakeBottom, "lakeBottom");
DEF(LevelType::MixingLayer, "mixingLayer");
DEF(LevelType::OceanModel, "oceanModel");
DEF(LevelType::OceanModelLayer, "oceanModelLayer");
DEF(LevelType::MixedLayerDepthByDensity, "mixedLayerDepthByDensity");
DEF(LevelType::MixedLayerDepthByTemperature, "mixedLayerDepthByTemperature");
DEF(LevelType::SnowLayerOverIceOnWater, "snowLayerOverIceOnWater");
DEF(LevelType::IceTopOnWater, "iceTopOnWater");
DEF(LevelType::IceLayerOnWater, "iceLayerOnWater");
DEF(LevelType::EntireMeltPond, "entireMeltPond");
DEF(LevelType::WaterSurfaceToIsothermalOceanLayer, "waterSurfaceToIsothermalOceanLayer");
DEF(LevelType::AbstractSingleLevel, "abstractSingleLevel");
DEF(LevelType::AbstractLevel, "abstractLevel");
DEF(LevelType::AbstractMultipleLevel, "abstractMultipleLevel");
DEF(LevelType::HeightAboveSeaAt10M, "heightAboveSeaAt10m");
DEF(LevelType::HeightAboveSeaAt2M, "heightAboveSeaAt2m");
DEF(LevelType::HeightAboveGroundAt10M, "heightAboveGroundAt10m");
DEF(LevelType::HeightAboveGroundAt2M, "heightAboveGroundAt2m");
DEF(LevelType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::concepts_
