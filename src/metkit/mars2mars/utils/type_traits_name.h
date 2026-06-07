/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file type_traits_name.h
/// @brief Human-readable type names used in diagnostics.
#pragma once

#include <cxxabi.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "metkit/codes/api/CodesTypes.h"
#include "metkit/mars2mars/utils/generalUtils.h"

namespace metkit::mars2mars::utils {

/// @brief Return a readable name for `T`.
// ============================================================
//  Primary template (deferred to fallback demangling)
// ============================================================

template <class T>
constexpr std::string_view type_name();


// ============================================================
//  Base type specializations
// ============================================================

template <>
constexpr std::string_view type_name<std::string>() {
    return "string";
}
template <>
constexpr std::string_view type_name<bool>() {
    return "bool";
}
template <>
constexpr std::string_view type_name<char>() {
    return "char";
}
template <>
constexpr std::string_view type_name<signed char>() {
    return "signed char";
}
template <>
constexpr std::string_view type_name<unsigned char>() {
    return "unsigned char";
}

template <>
constexpr std::string_view type_name<int>() {
    return "int";
}
template <>
constexpr std::string_view type_name<long>() {
    return "long";
}
template <>
constexpr std::string_view type_name<long long>() {
    return "long long";
}

template <>
constexpr std::string_view type_name<unsigned int>() {
    return "unsigned int";
}
template <>
constexpr std::string_view type_name<unsigned long>() {
    return "unsigned long";
}
template <>
constexpr std::string_view type_name<unsigned long long>() {
    return "unsigned long long";
}

template <>
constexpr std::string_view type_name<float>() {
    return "float";
}
template <>
constexpr std::string_view type_name<double>() {
    return "double";
}
template <>
constexpr std::string_view type_name<long double>() {
    return "long double";
}

// ============================================================
// std::vector<T> specializations
// ============================================================

template <>
constexpr std::string_view type_name<std::vector<std::string>>() {
    return "vector<string>";
}
template <>
constexpr std::string_view type_name<std::vector<bool>>() {
    return "vector<bool>";
}
template <>
constexpr std::string_view type_name<std::vector<char>>() {
    return "vector<char>";
}
template <>
constexpr std::string_view type_name<std::vector<signed char>>() {
    return "vector<signed char>";
}
template <>
constexpr std::string_view type_name<std::vector<unsigned char>>() {
    return "vector<unsigned char>";
}

template <>
constexpr std::string_view type_name<std::vector<int>>() {
    return "vector<int>";
}
template <>
constexpr std::string_view type_name<std::vector<long>>() {
    return "vector<long>";
}
template <>
constexpr std::string_view type_name<std::vector<long long>>() {
    return "vector<long long>";
}

template <>
constexpr std::string_view type_name<std::vector<unsigned int>>() {
    return "vector<unsigned int>";
}
template <>
constexpr std::string_view type_name<std::vector<unsigned long>>() {
    return "vector<unsigned long>";
}
template <>
constexpr std::string_view type_name<std::vector<unsigned long long>>() {
    return "vector<unsigned long long>";
}

template <>
constexpr std::string_view type_name<std::vector<float>>() {
    return "vector<float>";
}
template <>
constexpr std::string_view type_name<std::vector<double>>() {
    return "vector<double>";
}
template <>
constexpr std::string_view type_name<std::vector<long double>>() {
    return "vector<long double>";
}
template <>
constexpr std::string_view type_name<metkit::codes::Span<const float>>() {
    return "vector_view<float>";
}
template <>
constexpr std::string_view type_name<metkit::codes::Span<const double>>() {
    return "vector_view<double>";
}

}  // namespace metkit::mars2mars::utils
