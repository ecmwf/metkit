#pragma once

#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/mars2grib/backend/concepts/concept_core.h"


namespace metkit::mars2grib::backend::cnpts {

// ======================================================
// NAME OF THE CONCEPT
// ======================================================
inline constexpr std::string_view levelName{"level"};


// ======================================================
// ENUM OF VARIANTS FOR THIS CONCEPT
// ======================================================
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
    Hybrid,
    Theta,
    PotentialVorticity,
    SnowLayer,
    SoilLayer,
    SeaIceLayer,
    DepthBelowSeaLayer,
    LakeBottom,
    MixingLayer,
    IceTopOnWater,
    IceLayerOnWater,
    AbstractSingleLevel,
    AbstractMultipleLevel,
    HeightAboveSeaAt10M,
    HeightAboveSeaAt2M,
    HeightAboveGroundAt10M,
    HeightAboveGroundAt2M,
    Default
};

// ======================================================
// COMPILE-TIME TYPELIST
// ======================================================
using LevelList =
    ValueList<LevelType::Surface, LevelType::EntireAtmosphere, LevelType::EntireLake, LevelType::CloudBase,
              LevelType::Tropopause, LevelType::NominalTop, LevelType::MostUnstableParcel, LevelType::MixedLayerParcel,
              LevelType::Isothermal, LevelType::IsobaricInPa, LevelType::IsobaricInHpa, LevelType::LowCloudLayer,
              LevelType::MediumCloudLayer, LevelType::HighCloudLayer, LevelType::MeanSea, LevelType::HeightAboveSea,
              LevelType::HeightAboveGround, LevelType::Hybrid, LevelType::Theta, LevelType::PotentialVorticity,
              LevelType::SnowLayer, LevelType::SoilLayer, LevelType::SeaIceLayer, LevelType::DepthBelowSeaLayer,
              LevelType::LakeBottom, LevelType::MixingLayer, LevelType::IceTopOnWater, LevelType::IceLayerOnWater,
              LevelType::AbstractSingleLevel, LevelType::AbstractMultipleLevel, LevelType::HeightAboveSeaAt10M,
              LevelType::HeightAboveSeaAt2M, LevelType::HeightAboveGroundAt10M, LevelType::HeightAboveGroundAt2M,
              LevelType::Default>;

// ======================================================
// VARIANT -> STRING MAPPING
// ======================================================
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
DEF(LevelType::Hybrid, "hybrid");
DEF(LevelType::Theta, "theta");
DEF(LevelType::PotentialVorticity, "potentialVorticity");
DEF(LevelType::SnowLayer, "snowLayer");
DEF(LevelType::SoilLayer, "soilLayer");
DEF(LevelType::SeaIceLayer, "seaIceLayer");
DEF(LevelType::DepthBelowSeaLayer, "depthBelowSeaLayer");
DEF(LevelType::LakeBottom, "lakeBottom");
DEF(LevelType::MixingLayer, "mixingLayer");
DEF(LevelType::IceTopOnWater, "iceTopOnWater");
DEF(LevelType::IceLayerOnWater, "iceLayerOnWater");
DEF(LevelType::AbstractSingleLevel, "abstractSingleLevel");
DEF(LevelType::AbstractMultipleLevel, "abstractMultipleLevel");
DEF(LevelType::HeightAboveSeaAt10M, "heightAboveSeaAt10m");
DEF(LevelType::HeightAboveSeaAt2M, "heightAboveSeaAt2m");
DEF(LevelType::HeightAboveGroundAt10M, "heightAboveGroundAt10m");
DEF(LevelType::HeightAboveGroundAt2M, "heightAboveGroundAt2m");
DEF(LevelType::Default, "default");

#undef DEF

}  // namespace metkit::mars2grib::backend::cnpts
