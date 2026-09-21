/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file shapeOfTheEarth.h
/// @brief Resolve the Earth-shape code with its spherical default.
#pragma once
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct ShapeOfTheEarth {
    using value_type = long;
    static constexpr std::string_view name{"shapeOfTheEarth"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};
    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<long> resolve(const Misc& misc, const Mars& mars, const Opt& opt, Cntx& cntx) {
        return detail::resolve_scalar_default<long>(misc, mars, opt, name,
            [](const auto&, const auto&, auto&) { return 6L; }, cntx);
    }
};
}
