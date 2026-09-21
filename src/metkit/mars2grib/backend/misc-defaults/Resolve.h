/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Resolve.h
/// @brief Public typed entry points for misc resolution.

#pragma once

#include <optional>
#include <string>
#include <type_traits>

#include "metkit/mars2grib/backend/misc-defaults/Types.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults {

template <class Rule, class MiscDict_t, class MarsDict_t, class OptDict_t, class Cntx_t>
ResolvedMisc<typename Rule::value_type> resolve_misc_or_throw(const MiscDict_t& misc, const MarsDict_t& mars,
                                                              const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    static_assert(Rule::policy != MiscPolicy::OptionalOverride,
                  "Optional overrides must be resolved with resolve_misc_opt");
    try {
        auto result = Rule::resolve(misc, mars, opt, utils::profiling::callSite(cntx, Here()));
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMiscDefaultsException(
            "Unable to resolve misc property `" + std::string(Rule::name) + "`", Here()));
    }
    mars2gribUnreachable();
}

template <class Rule, class MiscDict_t, class MarsDict_t, class OptDict_t, class Cntx_t>
std::optional<ResolvedMisc<typename Rule::value_type>> resolve_misc_opt(
    const MiscDict_t& misc, const MarsDict_t& mars, const OptDict_t& opt, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    static_assert(Rule::policy != MiscPolicy::Mandatory,
                  "Mandatory misc properties must be resolved with resolve_misc_or_throw");
    try {
        auto result = Rule::resolve(misc, mars, opt, utils::profiling::callSite(cntx, Here()));
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
    catch (...) {
        std::throw_with_nested(utils::exceptions::Mars2GribMiscDefaultsException(
            "Unable to resolve optional misc property `" + std::string(Rule::name) + "`", Here()));
    }
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::misc_defaults
