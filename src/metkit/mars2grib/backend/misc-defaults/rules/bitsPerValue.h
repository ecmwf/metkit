/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file bitsPerValue.h
/// @brief Context-dependent default for GRIB packing precision.

#pragma once

#include <string>

#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
#include "metkit/mars2grib/utils/representationFamily.h"

namespace metkit::mars2grib::backend::misc_defaults::rules {

struct BitsPerValue {
    using value_type = long;
    static constexpr std::string_view name{"bitsPerValue"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};

    template <class Mars, class Opt, class Cntx>
    static long default_value(const Mars& mars, const Opt& opt, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        if (utils::resolve_representation_family_or_throw(mars, opt, utils::profiling::callSite(cntx, Here())) ==
            utils::RepresentationFamily::SphericalHarmonics) {
            utils::profiling::profileExitFunction(cntx, Here());
            return 16L;
        }

        const long param = utils::dict_traits::get_or_throw<long>(
            mars, "param", utils::profiling::callSite(cntx, Here()));
        const std::string levtype = utils::dict_traits::get_or_throw<std::string>(
            mars, "levtype", utils::profiling::callSite(cntx, Here()));
        const bool compression = utils::dict_traits::get_or_throw<bool>(
            opt, "enableBitsPerValueCompression", utils::profiling::callSite(cntx, Here()));

        long result = 16L;
        if (param == 248) result = 8L;
        else if (param == 141 || param == 228141 || param == 244) result = 24L;
        else if ((param == 246 || param == 247) && levtype == "pl") result = 12L;
        else if (param > 210000 && param < 228000) result = 24L;
        else if (param == 260510 || param == 260511) result = 10L;
        else if (compression && levtype == "ml") result = 10L;

        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }

    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<long> resolve(const Misc& misc, const Mars& mars, const Opt& opt, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        auto result = detail::resolve_scalar_default<long>(misc, mars, opt, name,
            [](const auto& m, const auto& o, auto& c) { return default_value(m, o, c); },
            utils::profiling::callSite(cntx, Here()));
        if (result.value < 0 || result.value > 64) {
            throw utils::exceptions::Mars2GribMiscDefaultsException(
                "Misc key `bitsPerValue` must be between 0 and 64", Here());
        }
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

}  // namespace metkit::mars2grib::backend::misc_defaults::rules
