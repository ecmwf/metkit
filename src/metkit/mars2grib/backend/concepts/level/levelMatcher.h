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
/// @file levelMatcher.h
/// @brief Entry-level matcher for the GRIB `level` concept.
///
/// This header defines the runtime matcher used by the concept registry to
/// map MARS `levtype`, `param`, and, where required, `levelist` metadata onto
/// GRIB level concept variants.
///
/// The implementation is split into levtype-specific helper functions in the
/// internal `impl` namespace and a public entry-level matcher. Each helper is
/// responsible for one MARS level type and returns a local `LevelType` variant
/// index.
///
/// The matcher follows the standard mars2grib matching contract:
/// - return a local concept variant index when the concept is active,
/// - return `compile_time_registry_engine::MISSING` when it is not active,
/// - wrap runtime failures as nested `Mars2GribMatcherException` instances.
///
/// Nested error handling is intentionally applied both at the entry point and
/// inside the levtype-specific helpers so diagnostics retain the full matcher
/// call chain and the levtype-specific mapping context.
///
/// @ingroup mars2grib_backend_concepts
///
#pragma once

// System include
#include <cstddef>
#include <exception>
#include <string>

// Utils
#include "metkit/mars2grib/backend/concepts/level/levelEnum.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/paramMatcher.h"

namespace metkit::mars2grib::backend::concepts_ {

namespace impl {

///
/// @brief Match surface-level MARS parameters.
///
/// Maps `levtype=sfc` parameters onto their GRIB level concept variant.
/// Wave-period parameters deliberately return `MISSING` because they are handled
/// by the wave concept rather than by level encoding.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index, or
/// `compile_time_registry_engine::MISSING` for surface products owned by another
/// concept.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no surface-level mapping exists. Lower-level exceptions are preserved
/// through `std::throw_with_nested`.
///
inline std::size_t matchSFC(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, 228023)) {
            return static_cast<std::size_t>(LevelType::CloudBase);
        }
        if (matchAny(param, 262118)) {
            return static_cast<std::size_t>(LevelType::DepthBelowSeaLayer);
        }
        if (matchAny(param, 59, 78, 79, 136, 137, 164, 194, 206, range(162059, 162063), 162071, 162072, 162093, 228001,
                     228044, 228050, 228052, range(228088, 228090), 228164, 235087, 235088, 235136, 235137, 235287,
                     235288, 235290, 235326, 235383, 237087, 237088, 237137, 237287, 237288, 237290, 237326, 238087,
                     238088, 238137, 238287, 238288, 238290, 238326, 239087, 239088, 239137, 239287, 239288, 239290,
                     239326, 260132)) {
            return static_cast<std::size_t>(LevelType::EntireAtmosphere);
        }
        if (matchAny(param, 228007, 228011)) {
            return static_cast<std::size_t>(LevelType::EntireLake);
        }
        if (matchAny(param, 129172)) {
            return static_cast<std::size_t>(LevelType::HeightAboveGround);
        }
        if (matchAny(param, 49, 123, 165, 166, 207, 228005, 228028, 228029, 228131, 228132, 235165, 235166, 237165,
                     237166, 237207, 237318, 238165, 238166, 238207, 239165, 239166, 239207, 260260)) {
            return static_cast<std::size_t>(LevelType::HeightAboveGroundAt10M);
        }
        if (matchAny(param, 121, 122, 167, 168, 201, 202, 174096, 228004, 228037, 235168, 237167, 237168, 238167,
                     238168, 239167, 239168, 260242)) {
            return static_cast<std::size_t>(LevelType::HeightAboveGroundAt2M);
        }
        if (matchAny(param, 140233, 140245, 140249, 141233, 141245, 143233, 143245, 144233, 144245, 145233, 145245)) {
            return static_cast<std::size_t>(LevelType::HeightAboveSeaAt10M);
        }
        if (matchAny(param, 188, 3075)) {
            return static_cast<std::size_t>(LevelType::HighCloudLayer);
        }
        if (matchAny(param, 228014, 235309, 237309, 238309, 239309)) {
            return static_cast<std::size_t>(LevelType::IceLayerOnWater);
        }
        if (matchAny(param, 228013)) {
            return static_cast<std::size_t>(LevelType::IceTopOnWater);
        }
        if (matchAny(param, 262104)) {
            return static_cast<std::size_t>(LevelType::Isothermal);
        }
        if (matchAny(param, 228010, 235305, 237305, 238305, 239305)) {
            return static_cast<std::size_t>(LevelType::LakeBottom);
        }
        if (matchAny(param, 186, 3073, 235108, 237108, 238108, 239108)) {
            return static_cast<std::size_t>(LevelType::LowCloudLayer);
        }
        if (matchAny(param, 151, 235151, 237151, 238151, 239151)) {
            return static_cast<std::size_t>(LevelType::MeanSea);
        }
        if (matchAny(param, 187, 3074)) {
            return static_cast<std::size_t>(LevelType::MediumCloudLayer);
        }
        if (matchAny(param, range(228231, 228234))) {
            return static_cast<std::size_t>(LevelType::MixedLayerParcel);
        }
        if (matchAny(param, 228008, 228009, 235090, 235091, 237090, 237091, 238090, 238091, 239090, 239091)) {
            return static_cast<std::size_t>(LevelType::MixingLayer);
        }
        if (matchAny(param, range(228235, 228237))) {
            return static_cast<std::size_t>(LevelType::MostUnstableParcel);
        }
        if (matchAny(param, 178, 179, 208, 209, 212, 235039, 235040, 235049, 235050, 235053)) {
            return static_cast<std::size_t>(LevelType::NominalTop);
        }
        if (matchAny(param, 263024, 265024, 266024, 267024)) {
            return static_cast<std::size_t>(LevelType::SeaIceLayer);
        }
        if (matchAny(param, 235077, 235094, 237077, 237094, 238077, 238094, 239077, 239094)) {
            return static_cast<std::size_t>(LevelType::SoilLayer);
        }
        if (matchAny(param, 8, 9, range(15, 18), 20, range(26, 45), 47, 50, 57, 58, 66, 67, 74, 129, 134, 139,
                     range(141, 148), range(159, 163), 169, 170, range(172, 177), range(180, 182), 189, range(195, 198),
                     205, 210, 211, 213, range(228, 232), range(234, 236), range(238, 240), range(243, 245), 3020, 3062,
                     3067, 3099, range(131074, 131077), range(140098, 140105), 140112, 140113, range(140121, 140129),
                     range(140131, 140134), range(140207, 140209), 140211, 140212, range(140214, 140232),
                     range(140234, 140239), 140244, range(140246, 140248), range(140252, 140254), range(141101, 141105),
                     141208, 141209, 141215, 141216, 141220, 141229, 141232, range(143101, 143105), 143208, 143209,
                     143215, 143216, 143220, 143229, 143232, range(144101, 144105), 144208, 144209, 144215, 144216,
                     144220, 144229, 144232, range(145101, 145105), 145208, 145209, 145215, 145216, 145220, 145229,
                     145232, 160198, range(162100, 162113), 200199, range(210186, 210191), range(210198, 210202),
                     range(210260, 210264), range(222001, 222256), 228002, 228003, 228012, range(228015, 228022),
                     228024, 228026, 228027, 228032, 228035, 228036, range(228046, 228048), 228051, 228053,
                     range(228057, 228060), 228129, 228130, 228141, 228143, 228144, range(228216, 228228), 228251,
                     229001, 229007, range(231001, 231003), 231005, 231010, 231012, 231057, 231058,
                     range(233000, 233031), 235020, 235021, range(235029, 235031), range(235033, 235038),
                     range(235041, 235043), 235048, 235051, 235052, 235055, 235058, range(235078, 235080), 235083,
                     235084, 235093, 235134, 235159, 235189, 235263, 235283, 235339, 237013, 237041, 237042, 237055,
                     237078, 237080, 237083, 237084, 237093, 237117, 237134, 237159, 237263, 237321, 238013, 238041,
                     238042, 238055, 238078, 238080, 238083, 238084, 238093, 238134, 238159, 238263, 239041, 239042,
                     239078, 239080, 239083, 239084, 239093, 239134, 239159, 239263, 260004, 260005, 260015, 260038,
                     260048, 260109, 260121, 260123, 260255, 260259, 260289, 260292, 260293, range(260318, 260321),
                     260338, 260339, 260509, 260682, 260683, 260688, 261001, 261002, range(261014, 261016), 261018,
                     261023, 262000, 262100, 262124, 262139, 262140, 262144)) {
            return static_cast<std::size_t>(LevelType::Surface);
        }
        if (matchAny(param, 228045, 235322, 237322, 238322, 239322)) {
            return static_cast<std::size_t>(LevelType::Tropopause);
        }

        // Chemical
        if (matchAny(param, range(228080, 228085), range(233032, 233035), range(235062, 235064))) {
            return static_cast<std::size_t>(LevelType::Surface);
        }

        // Wave period
        if (matchAny(param, range(140114, 140120))) {
            return compile_time_registry_engine::MISSING;
        }

        // ECMWF covariance paramIds (254001..254017) are defined in
        // eccodes/definitions/grib2/localConcepts/{ecmf,era6}/paramId.def with
        // typeOfFirstFixedSurface=254, which maps to the eccodes typeOfLevel
        // concept "abstractLevel".
        if (matchAny(param, range(254001, 254017))) {
            return static_cast<std::size_t>(LevelType::AbstractLevel);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype SFC", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "sfc", "Unable to match `level` concept for levtype \"sfc\"", Here()));
    }
}

///
/// @brief Match height-level MARS parameters.
///
/// Maps `levtype=hl` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no height-level mapping exists. Lower-level exceptions are preserved
/// through `std::throw_with_nested`.
///
inline std::size_t matchHL(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, 10, 54, range(130, 132), 157, 246, 247, 3031, 235097, 235131, 235132, 237097, 237131,
                     237132, 238097, 238131, 238132, 239097, 239131, 239132)) {
            return static_cast<std::size_t>(LevelType::HeightAboveGround);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype HL", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "hl", "Unable to match `level` concept for levtype \"hl\"", Here()));
    }
}

///
/// @brief Match model-level MARS parameters.
///
/// Maps `levtype=ml` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no model-level mapping exists. Lower-level exceptions are preserved
/// through `std::throw_with_nested`.
///
inline std::size_t matchML(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        // Single-level subset of ML params: 2D fields published on the
        // model-level levtype but not requiring a vertical PV array. This
        // guard fires before the multi-level rule below; params listed here
        // are removed from the multi-level set.
        if (matchAny(param, 22, 127, 128, 129, 152)) {
            return static_cast<std::size_t>(LevelType::ModelSingleLevel);
        }

        // Multi-level model fields: full vertical column, require allocation
        // and population of the PV array describing the hybrid coordinate.
        if (matchAny(param, 21, 23, range(75, 77), range(130, 133), 135, 138, range(155, 157), 203, range(246, 248),
                     range(162100, 162113), 260290, 260292, 260293)) {
            return static_cast<std::size_t>(LevelType::ModelMultipleLevel);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype ML", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "ml", "Unable to match `level` concept for levtype \"ml\"", Here()));
    }
}

///
/// @brief Match pressure-level MARS parameters.
///
/// Maps `levtype=pl` parameters onto pressure-level GRIB variants. The MARS
/// `levelist` value determines whether the pressure level is interpreted in hPa
/// or Pa.
///
/// @param[in] param MARS parameter identifier
/// @param[in] level MARS pressure level value from `levelist`
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no pressure-level mapping exists. Lower-level exceptions are preserved
/// through `std::throw_with_nested`.
///
inline std::size_t matchPL(const long param, const long level) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, 1, 2, 10, 60, 75, 76, range(129, 135), 138, 152, range(155, 157), 203, range(246, 248),
                     235100, range(235129, 235133), 235135, 235138, 235152, 235155, 235157, 235203, 235246, 260290,
                     263107)) {
            if (level >= 100) {
                return static_cast<std::size_t>(LevelType::IsobaricInHpa);
            }
            else {
                return static_cast<std::size_t>(LevelType::IsobaricInPa);
            }
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype PL", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "pl",
            "Unable to match `level` concept for levtype \"pl\" and levelist \"" + std::to_string(level) + "\"",
            Here()));
    }
}

///
/// @brief Match potential-temperature-level MARS parameters.
///
/// Maps `levtype=pt` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no potential-temperature-level mapping exists. Lower-level exceptions are
/// preserved through `std::throw_with_nested`.
///
inline std::size_t matchPT(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, 53, 54, 60, range(131, 133), 138, 155, 203, 235100, 235203, 237203, 238203, 239203)) {
            return static_cast<std::size_t>(LevelType::Theta);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype PT", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "pt", "Unable to match `level` concept for levtype \"pt\"", Here()));
    }
}


///
/// @brief Match potential-vorticity-level MARS parameters.
///
/// Maps `levtype=pv` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no potential-vorticity-level mapping exists. Lower-level exceptions are
/// preserved through `std::throw_with_nested`.
///
inline std::size_t matchPV(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, 3, 54, 129, range(131, 133), 203, 235098, 235269)) {
            return static_cast<std::size_t>(LevelType::PotentialVorticity);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype PV", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "pv", "Unable to match `level` concept for levtype \"pv\"", Here()));
    }
}

///
/// @brief Match soil-level MARS parameters.
///
/// Maps `levtype=sol` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no soil-level mapping exists. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
inline std::size_t matchSOL(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, 262000, 262024)) {
            return static_cast<std::size_t>(LevelType::SeaIceLayer);
        }
        if (matchAny(param, 33, 74, 238, 228038, 228141, 235078, 235080, 237080, 238080, 239080)) {
            return static_cast<std::size_t>(LevelType::SnowLayer);
        }
        if (matchAny(param, 183, 235077, 260199, 260360)) {
            return static_cast<std::size_t>(LevelType::SoilLayer);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype SOL", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "sol", "Unable to match `level` concept for levtype \"sol\"", Here()));
    }
}

///
/// @brief Match abstract-level MARS parameters.
///
/// Maps `levtype=al` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no abstract-level mapping exists. Lower-level exceptions are preserved
/// through `std::throw_with_nested`.
///
inline std::size_t matchAL(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, range(213101, 213160))) {
            return static_cast<std::size_t>(LevelType::AbstractSingleLevel);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype AL", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "al", "Unable to match `level` concept for levtype \"al\"", Here()));
    }
}

///
/// @brief Match two-dimensional ocean-level MARS parameters.
///
/// Maps `levtype=o2d` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no two-dimensional ocean-level mapping exists. Lower-level exceptions are
/// preserved through `std::throw_with_nested`.
///
inline std::size_t matchO2D(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, 262000, 262003, 262004, 262008, 262014, 262023)) {
            return static_cast<std::size_t>(LevelType::IceLayerOnWater);
        }
        if (matchAny(param, 262001, 262005, 262006, 262906, 262907)) {
            return static_cast<std::size_t>(LevelType::IceTopOnWater);
        }
        if (matchAny(param, 262002, 262009, 262011, 262015)) {
            return static_cast<std::size_t>(LevelType::SnowLayerOverIceOnWater);
        }
        if (matchAny(param, 262017, 262018)) {
            return static_cast<std::size_t>(LevelType::EntireMeltPond);
        }
        if (matchAny(param, 262100, 262101, range(262108, 262112), 262124, 262125, 262130, 262139, 262140, 262143,
                     262900)) {
            return static_cast<std::size_t>(LevelType::OceanSurface);
        }
        if (matchAny(param, range(262102, 262106))) {
            return static_cast<std::size_t>(LevelType::Isothermal);
        }
        if (matchAny(param, range(262113, 262115))) {
            return static_cast<std::size_t>(LevelType::MixedLayerDepthByDensity);
        }
        if (matchAny(param, 262116)) {
            return static_cast<std::size_t>(LevelType::MixedLayerDepthByTemperature);
        }
        if (matchAny(param, 262118, 262119, 262121, 262122, 262146, 262147)) {
            return static_cast<std::size_t>(LevelType::DepthBelowSeaLayer);
        }
        if (matchAny(param, 262120, 262123, 262148)) {
            return static_cast<std::size_t>(LevelType::OceanSurfaceToBottom);
        }
        if (matchAny(param, 262141)) {
            return static_cast<std::size_t>(LevelType::WaterSurfaceToIsothermalOceanLayer);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype O2D", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "o2d", "Unable to match `level` concept for levtype \"o2d\"", Here()));
    }
}

///
/// @brief Match three-dimensional ocean-level MARS parameters.
///
/// Maps `levtype=o3d` parameters onto their GRIB level concept variant.
///
/// @param[in] param MARS parameter identifier
///
/// @return Local `LevelType` variant index.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If no three-dimensional ocean-level mapping exists. Lower-level exceptions
/// are preserved through `std::throw_with_nested`.
///
inline std::size_t matchO3D(const long param) {
    try {
        using metkit::mars2grib::util::param_matcher::matchAny;
        using metkit::mars2grib::util::param_matcher::range;

        if (matchAny(param, range(262500, 262502), 262505, 262506)) {
            return static_cast<std::size_t>(LevelType::OceanModelLayer);
        }
        if (matchAny(param, 262507)) {
            return static_cast<std::size_t>(LevelType::OceanModel);
        }

        throw utils::exceptions::Mars2GribMatcherException(
            "No mapping exists for param \"" + std::to_string(param) + "\" on levtype O3D", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException(
            param, "o3d", "Unable to match `level` concept for levtype \"o3d\"", Here()));
    }
}

}  // namespace impl

///
/// @brief Match the `level` concept variant.
///
/// The matcher first excludes products owned by other concepts, such as wave
/// spectra and satellite products. For remaining products, it reads `param` and
/// `levtype` and dispatches to the corresponding levtype-specific helper.
/// Pressure-level products additionally require `levelist`.
///
/// @tparam MarsDict_t Type of the MARS input dictionary
/// @tparam OptDict_t  Type of the options dictionary
///
/// @param[in] mars MARS input dictionary
/// @param[in] opt  Options dictionary
///
/// @return Local level variant index, or
/// `compile_time_registry_engine::MISSING` when the level concept is inactive.
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribMatcherException
/// If required MARS metadata is missing, `levtype` is unsupported, no mapping
/// exists for the `(levtype, param)` combination, or lower-level matcher
/// evaluation fails. Lower-level exceptions are preserved through
/// `std::throw_with_nested`.
///
template <class MarsDict_t, class OptDict_t>
std::size_t levelMatcher(const MarsDict_t& mars, const OptDict_t& opt) {
    try {
        using metkit::mars2grib::utils::dict_traits::get_or_throw;
        using metkit::mars2grib::utils::dict_traits::has;

        // Skip wave spectra and satellite products
        if ((has(mars, "frequency") && has(mars, "direction")) ||                    // Wave spectra
            (has(mars, "channel") && has(mars, "ident") && has(mars, "instrument"))  // Satellite
        ) {
            return compile_time_registry_engine::MISSING;
        }

        const auto param   = get_or_throw<long>(mars, "param");
        const auto levtype = get_or_throw<std::string>(mars, "levtype");

        if (levtype == "sfc") {
            return impl::matchSFC(param);
        }
        if (levtype == "hl") {
            return impl::matchHL(param);
        }
        if (levtype == "ml") {
            return impl::matchML(param);
        }
        if (levtype == "pl") {
            const auto level = get_or_throw<long>(mars, "levelist");
            return impl::matchPL(param, level);
        }
        if (levtype == "pt") {
            return impl::matchPT(param);
        }
        if (levtype == "pv") {
            return impl::matchPV(param);
        }
        if (levtype == "sol") {
            return impl::matchSOL(param);
        }
        if (levtype == "al") {
            return impl::matchAL(param);
        }
        if (levtype == "o2d") {
            return impl::matchO2D(param);
        }
        if (levtype == "o3d") {
            return impl::matchO3D(param);
        }

        throw utils::exceptions::Mars2GribMatcherException("Unknown levtype \"" + levtype + "\"", Here());
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMatcherException("Unable to match `level` concept", Here()));
    }
};

}  // namespace metkit::mars2grib::backend::concepts_
