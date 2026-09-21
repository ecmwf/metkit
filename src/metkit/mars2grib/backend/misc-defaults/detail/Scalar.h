/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Scalar.h
/// @brief Reusable mechanics for scalar misc rules.

#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "metkit/mars2grib/backend/misc-defaults/Access.h"
#include "metkit/mars2grib/backend/misc-defaults/Types.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults::detail {

template <class T, class MiscDict_t, class MarsDict_t, class OptDict_t, class Default_t, class Cntx_t>
ResolvedMisc<T> resolve_scalar_default(const MiscDict_t& misc, const MarsDict_t& mars, const OptDict_t& opt,
                                      std::string_view key, Default_t&& defaultValue, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    if (contains(misc, key, utils::profiling::callSite(cntx, Here()))) {
        ResolvedMisc<T> result{read_present_or_throw<T>(misc, key, utils::profiling::callSite(cntx, Here())),
                               MiscSource::Explicit, key};
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
    ResolvedMisc<T> result{std::forward<Default_t>(defaultValue)(mars, opt,
                                                                 utils::profiling::callSite(cntx, Here())),
                           MiscSource::Default, key};
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class T, class MiscDict_t, class Cntx_t>
std::optional<ResolvedMisc<T>> resolve_scalar_optional(const MiscDict_t& misc, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    std::optional<ResolvedMisc<T>> result;
    if (contains(misc, key, utils::profiling::callSite(cntx, Here()))) {
        result = ResolvedMisc<T>{read_present_or_throw<T>(misc, key, utils::profiling::callSite(cntx, Here())),
                                 MiscSource::Explicit, key};
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class T, class MarsDict_t, class OptDict_t, class Cntx_t>
T fallback_not_implemented(const MarsDict_t&, const OptDict_t&, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    throw utils::exceptions::Mars2GribMiscDefaultsException(
        "TODO: fallback not implemented for mandatory misc property `" + std::string(key) + "`", Here());
}

}  // namespace metkit::mars2grib::backend::misc_defaults::detail
