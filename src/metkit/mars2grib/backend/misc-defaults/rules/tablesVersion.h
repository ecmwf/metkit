/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */
/// @file tablesVersion.h
/// @brief Resolve the mandatory custom `tablesVersion` misc value.
#pragma once
#include "metkit/mars2grib/backend/misc-defaults/detail/Scalar.h"
namespace metkit::mars2grib::backend::misc_defaults::rules {
struct TablesVersion {
    using value_type = long;
    static constexpr std::string_view name{"tablesVersion"};
    static constexpr MiscPolicy policy{MiscPolicy::Mandatory};
    template <class Misc, class Mars, class Opt, class Cntx>
    static ResolvedMisc<long> resolve(const Misc& misc, const Mars& mars, const Opt& opt, Cntx& cntx) {
        return detail::resolve_scalar_default<long>(misc, mars, opt, name,
            [](const auto& m, const auto& o, auto& c) { return detail::fallback_not_implemented<long>(m, o, name, c); }, cntx);
    }
};
}
