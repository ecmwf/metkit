/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Access.h
/// @brief Sole low-level misc dictionary access boundary.

#pragma once

#include <optional>
#include <string_view>

#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::backend::misc_defaults::detail {

template <class MiscDict_t, class Cntx_t>
bool contains(const MiscDict_t& misc, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const bool result = utils::dict_traits::has(misc, key, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class T, class MiscDict_t, class Cntx_t>
T read_present_or_throw(const MiscDict_t& misc, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    T result = utils::dict_traits::get_or_throw<T>(misc, key, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

/// Attempt a typed read without interpreting absence or selecting a fallback.
///
/// This operation is reserved for representations that accept more than one
/// physical type. Callers must first use `contains()` so a malformed present
/// value cannot be mistaken for an absent key.
template <class T, class MiscDict_t, class Cntx_t>
std::optional<T> try_read_present(const MiscDict_t& misc, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    auto result = utils::dict_traits::get_opt<T>(misc, key, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class T, class MiscDict_t, class Cntx_t>
std::optional<T> read_optional(const MiscDict_t& misc, std::string_view key, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    std::optional<T> result;
    if (contains(misc, key, utils::profiling::callSite(cntx, Here()))) {
        result = read_present_or_throw<T>(misc, key, utils::profiling::callSite(cntx, Here()));
    }
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::backend::misc_defaults::detail
