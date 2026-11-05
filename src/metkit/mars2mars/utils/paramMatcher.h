// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file paramMatcher.h
/// @brief Helpers for compact parameter matching in rule tables.
#pragma once

namespace metkit::mars2mars::util::param_matcher {

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

}  // namespace metkit::mars2mars::util::param_matcher
