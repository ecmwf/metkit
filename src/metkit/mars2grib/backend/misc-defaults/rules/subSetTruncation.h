/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file subSetTruncation.h
/// @brief Context-dependent spectral subset truncation default.

#pragma once

#include <algorithm>
#include <memory>
#include <string>

#include "eckit/geo/Grid.h"
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"

namespace metkit::mars2grib::backend::misc_defaults::rules {

struct SubSetTruncation {
    using value_type = long;
    static constexpr std::string_view name{"subSetTruncation"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};

    template <class Mars, class Opt, class Cntx>
    static long truncation(const Mars& mars, const Opt& opt, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        if (utils::dict_traits::get_or_throw<bool>(
                opt, "skipSection3", utils::profiling::callSite(cntx, Here()))) {
            const auto grid = utils::dict_traits::get_or_throw<std::string>(
                mars, "grid", utils::profiling::callSite(cntx, Here()));
            const std::unique_ptr<const eckit::geo::Grid> gridSpec(
                eckit::geo::GridFactory::make_from_string(grid));
            const long result = static_cast<long>(gridSpec->truncation());
            utils::profiling::profileExitFunction(cntx, Here());
            return result;
        }
        const long result = utils::dict_traits::get_or_throw<long>(
            mars, "truncation", utils::profiling::callSite(cntx, Here()));
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }

    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<long> resolve(const Misc& misc, const Mars& mars, const Opt& opt, Cntx& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        const long maximum = truncation(mars, opt, utils::profiling::callSite(cntx, Here()));
        auto result = detail::resolve_scalar_default<long>(misc, mars, opt, name,
            [maximum](const auto&, const auto&, auto&) { return maximum >= 213 ? 20L : std::min(10L, maximum); },
            utils::profiling::callSite(cntx, Here()));
        if (maximum < 0 || result.value < 0 || result.value > maximum) {
            throw utils::exceptions::Mars2GribMiscDefaultsException(
                "Misc key `subSetTruncation` is outside the valid truncation range", Here());
        }
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
};

}  // namespace metkit::mars2grib::backend::misc_defaults::rules
