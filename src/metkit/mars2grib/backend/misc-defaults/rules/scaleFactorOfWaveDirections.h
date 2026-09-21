/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file scaleFactorOfWaveDirections.h
/// @brief Resolve the wave-direction scale factor with its default of 2.
#pragma once
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct ScaleFactorOfWaveDirections {
    using value_type = long;
    static constexpr std::string_view name{"scaleFactorOfWaveDirections"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};
    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<long> resolve(const Misc& misc, const Mars& mars, const Opt& opt, Cntx& cntx) {
        return detail::resolve_scalar_default<long>(misc, mars, opt, name,
            [](const auto&, const auto&, auto&) { return 2L; }, cntx);
    }
};
}
