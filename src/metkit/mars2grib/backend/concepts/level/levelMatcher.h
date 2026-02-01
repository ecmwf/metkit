/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 *
 * NOTE: This file is machine generated!
 */
#pragma once

#include <string>
#include <iostream>

#include "metkit/mars2grib/backend/concepts/level/levelEnum.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"


namespace metkit::mars2grib::backend::concepts_ {

#if 0
namespace levtype {

template <class MarsDict_t>
std::size_t match_sfc(const MarsDict_t& mars ) {

using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;


    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, 228023)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::CloudBase );
    }
    else if (matchAny(param, 262118)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::DepthBelowSeaLayer );
    }
    else if (matchAny(param, 59, 78, 79, 136, 137, 164, 206, range(162059, 162063), 162071, 162072, 162093, 228001,
                      228044, 228050, 228052, range(228088, 228090), 228164, 235087, 235088, 235136, 235137, 235287,
                      235288, 235290, 235326, 235383, 237087, 237088, 237137, 237287, 237288, 237290, 237326, 238087,
                      238088, 238137, 238287, 238288, 238290, 238326, 239087, 239088, 239137, 239287, 239288, 239290,
                      239326, 260132)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::EntireAtmosphere );
    }
    else if (matchAny(param, 228007, 228011)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::EntireLake );
    }
    else if (matchAny(param, 129172)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::HeightAboveGround );
    }
    else if (matchAny(param, 49, 123, 165, 166, 207, 228005, 228028, 228029, 228131, 228132, 235165, 235166, 237165,
                      237166, 237207, 237318, 238165, 238166, 238207, 239165, 239166, 239207, 260260)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::HeightAboveGroundAt10M );
    }
    else if (matchAny(param, 121, 122, 167, 168, 201, 202, 174096, 228004, 228037, 235168, 237167, 237168, 238167,
                      238168, 239167, 239168, 260242)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::HeightAboveGroundAt2M );
    }
    else if (matchAny(param, 140233, 140245, 140249, 141233, 141245, 143233, 143245, 144233, 144245, 145233, 145245)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::HeightAboveSeaAt10M );
    }
    else if (matchAny(param, 188, 3075)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::HighCloudLayer );
    }
    else if (matchAny(param, 228014, 235309, 237309, 238309, 239309)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::IceLayerOnWater );
    }
    else if (matchAny(param, 228013)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::IceTopOnWater );
    }
    else if (matchAny(param, 262104)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::Isothermal );
    }
    else if (matchAny(param, 228010, 235305, 237305, 238305, 239305)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::LakeBottom );
    }
    else if (matchAny(param, 186, 3073, 235108, 237108, 238108, 239108)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::LowCloudLayer );
    }
    else if (matchAny(param, 151, 235151, 237151, 238151, 239151)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::MeanSea );
    }
    else if (matchAny(param, 187, 3074)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::MediumCloudLayer );
    }
    else if (matchAny(param, range(228231, 228234))) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::MixedLayerParcel );
    }
    else if (matchAny(param, 228008, 228009, 235090, 235091, 237090, 237091, 238090, 238091, 239090, 239091)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::MixingLayer );
    }
    else if (matchAny(param, range(228235, 228237))) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::MostUnstableParcel );
    }
    else if (matchAny(param, 178, 179, 208, 209, 212, 235039, 235040, 235049, 235050, 235053)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::NominalTop );
    }
    else if (matchAny(param, 263024, 265024, 266024, 267024)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::SeaIceLayer );
    }
    else if (matchAny(param, 235077, 235094, 237077, 237094, 238077, 238094, 239077, 239094)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::SoilLayer );
    }
    else if (matchAny(
                 param, 8, 9, range(15, 18), 20, range(26, 45), 47, 50, 57, 58, 66, 67, 74, 129, 134, 139,
                 range(141, 148), range(159, 163), 169, 170, range(172, 177), range(180, 182), 189, range(195, 198),
                 205, 210, 211, 213, range(228, 232), range(234, 236), range(238, 240), range(243, 245), 3020, 3062,
                 3067, 3099, range(140098, 140105), 140112, 140113, range(140121, 140129), range(140131, 140134),
                 range(140207, 140209), 140211, 140212, range(140214, 140232), range(140234, 140239), 140244,
                 range(140246, 140248), range(140252, 140254), range(141101, 141105), 141208, 141209, 141215, 141216,
                 141220, 141229, 141232, range(143101, 143105), 143208, 143209, 143215, 143216, 143220, 143229, 143232,
                 range(144101, 144105), 144208, 144209, 144215, 144216, 144220, 144229, 144232, range(145101, 145105),
                 145208, 145209, 145215, 145216, 145220, 145229, 145232, 160198, range(162100, 162113), 200199,
                 range(210186, 210191), range(210200, 210202), range(210262, 210264), range(222001, 222256), 228003,
                 228012, range(228015, 228022), 228024, 228026, 228027, 228032, 228035, 228036, range(228046, 228048),
                 228051, 228053, range(228057, 228060), 228129, 228130, 228141, 228143, 228144, range(228216, 228228),
                 228251, range(231001, 231003), 231005, 231010, 231012, 231057, 231058, range(233000, 233031), 235020,
                 235021, range(235029, 235031), range(235033, 235038), range(235041, 235043), 235048, 235051, 235052,
                 235055, 235058, range(235078, 235080), 235083, 235084, 235093, 235134, 235159, 235189, 235263, 235283,
                 235339, 237013, 237041, 237042, 237055, 237078, 237080, 237083, 237084, 237093, 237117, 237134, 237159,
                 237263, 237321, 238013, 238041, 238042, 238055, 238078, 238080, 238083, 238084, 238093, 238134, 238159,
                 238263, 239041, 239042, 239078, 239080, 239083, 239084, 239093, 239134, 239159, 239263, 260004, 260005,
                 260015, 260038, 260048, 260109, 260121, 260123, 260255, 260259, 260289, 260292, 260293,
                 range(260318, 260321), 260338, 260339, 260509, 260682, 260683, 260688, 261001, 261002,
                 range(261014, 261016), 261018, 262000, 262100, 262124, 262139, 262140, 262144)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::Surface );
    }
    else if (matchAny(param, 228045, 235322, 237322, 238322, 239322)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::Tropopause );
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "sfc", "Mapping undefined for level", Here()};
    }

    return levelVariantIndex;
};

template <class MarsDict_t>
std::size_t match_hl(const MarsDict_t& mars) {

    
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, 10, 54, range(130, 132), 157, 246, 247, 3031, 235097, 235131, 235132, 237097, 237131, 237132,
                 238097, 238131, 238132, 239097, 239131, 239132)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::HeightAboveGround );
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "hl", "Mapping undefined for level", Here()};
    }
};

template <class MarsDict_t>
std::size_t match_ml(const MarsDict_t& mars) {

    
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, range(21, 23), range(75, 77), range(129, 133), 135, 138, 152, range(155, 157), 203,
                 range(246, 248), range(162100, 162113), 260290, 260292, 260293)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::Hybrid );
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "ml", "Mapping undefined for level", Here()};
    }

    return levelVariantIndex;
};

template <class MarsDict_t>
std::size_t match_pl(const MarsDict_t& mars) {

    
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;

    const auto param = get_or_throw<long>(mars, "param");
    const auto level = get_or_throw<long>(mars, "levelist");

    if (matchAny(param, 1, 2, 10, 60, 75, 76, range(129, 135), 138, 152, 155, 157, 203, range(246, 248), 235100,
                 range(235129, 235133), 235135, 235138, 235152, 235155, 235157, 235203, 235246, 260290, 263107)) {
        if (level >= 100) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::IsobaricInHpa );
        }
        else {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::IsobaricInPa );
        }
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "pl", "Mapping undefined for level", Here()};
    }

    return levelVariantIndex;
};

template <class MarsDict_t>
std::size_t match_pt(const MarsDict_t& mars) {

    
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, 53, 54, 60, range(131, 133), 138, 155, 203, 235100, 235203, 237203, 238203, 239203)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::Theta );
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "pt", "Mapping undefined for level", Here()};
    }

    return levelVariantIndex;
};

template <class MarsDict_t>
std::size_t match_pv(const MarsDict_t& mars) {

    
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, 3, 54, 129, range(131, 133), 203, 235098, 235269)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::PotentialVorticity );
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "pv", "Mapping undefined for level", Here()};
    }

    return levelVariantIndex;
};

template <class MarsDict_t>
std::size_t match_sol(const MarsDict_t& mars) {

    
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, 262000, 262024)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::SeaIceLayer );
    }
    else if (matchAny(param, 33, 74, 238, 228038, 228141, 235078, 235080, 237080, 238080, 239080)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::SnowLayer );
    }
    else if (matchAny(param, 183, 235077, 260199, 260360)) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::SoilLayer );
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "sol", "Mapping undefined for level", Here()};
    }

    return levelVariantIndex;
};

template <class MarsDict_t>
std::size_t match_al(const MarsDict_t& mars) {

    
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    std::size_t levelVariantIndex = GeneralRegistry::invalid;

    const auto param = get_or_throw<long>(mars, "param");

    if (matchAny(param, range(213101, 213160))) {
        levelVariantIndex = GeneralRegistry::globalIndex( LevelType::AbstractSingleLevel );
    }
    else {
        throw exceptions::Mars2GribMatcherException{param, "al", "Mapping undefined for level", Here()};
    }

    return levelVariantIndex;
};

}  // namespace levtype
#endif

template <class MarsDict_t, class OptDict_t>
std::size_t levelMatcher(const MarsDict_t& mars, const OptDict_t& opt ) {

#if 0
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using Fn  = std::size_t (*)(const MarsDict_t& );

    static const std::unordered_map<std::string_view, Fn> table = {
        {"sfc", &levtype::match_sfc<MarsDict_t>}, {"hl", &levtype::match_hl<MarsDict_t>},
        {"ml", &levtype::match_ml<MarsDict_t>},   {"pl", &levtype::match_pl<MarsDict_t>},
        {"pt", &levtype::match_pt<MarsDict_t>},   {"pv", &levtype::match_pv<MarsDict_t>},
        {"sol", &levtype::match_sol<MarsDict_t>}, {"al", &levtype::match_al<MarsDict_t>},
    };

    try {
        const std::string levtype = get_or_throw<std::string>(mars, "levtype");
        if (auto it = table.find(levtype); it != table.end()) {
            return it->second(mars, opt);
        }
        else {
            throw exceptions::Mars2GribMatcherException{levtype, "Mapping undefined for level", Here()};
        }
    }
    catch {
        std::throw_with_nested(exceptions::Mars2GribMatcherException{"Unable to match level", Here()});
    };

#endif

    std::cout << " - level matcher" << std::endl;

    return 0;

};

}  // namespace metkit::mars2grib::frontend::matchers
