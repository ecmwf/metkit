/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file periodItMin.h
/// @brief Resolve the optional lower wave-period limit.
#pragma once
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct PeriodItMin {
    using value_type = long;
    static constexpr std::string_view name{"iTmin"};
    static constexpr MiscPolicy policy{MiscPolicy::Optional};
    template <class Misc, class Mars, class Opt, class Cntx>
    static std::optional<ResolvedMisc<long>> resolve(const Misc& misc, const Mars&, const Opt&, Cntx& cntx) {
        return detail::resolve_scalar_optional<long>(misc, name, cntx);
    }
};
}
