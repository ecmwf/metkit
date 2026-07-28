/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file paramMatcher.h
/// @brief Helpers for compact parameter matching in rule tables.
#pragma once

namespace metkit::grib2mars::util::param_matcher {

/// @brief Inclusive integer range used by parameter predicates.
struct Range {
    int first;
    int last;
    /// @brief Check whether a value lies inside the range.
    bool contains(int x) const { return x >= first && x <= last; }
};

/// @brief Construct an inclusive integer range.
inline Range range(int first, int last) {
    return {first, last};
}

/// @brief Match a value against a range argument.
inline bool matchSingle(int x, const Range& arg) {
    return arg.contains(x);
}

/// @brief Match a value against a scalar argument.
inline bool matchSingle(int x, int y) {
    return x == y;
}

/// @brief Match a value against any supplied argument.
template <typename... T>
bool matchAny(int value, T... arg) {
    return (matchSingle(value, arg) || ...);
}

}  // namespace metkit::grib2mars::util::param_matcher
