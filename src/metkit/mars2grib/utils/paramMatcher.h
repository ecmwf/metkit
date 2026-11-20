// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

namespace metkit::mars2grib::util::param_matcher {

struct Range {
    int first;
    int last;
    bool contains(int x) const { return x >= first && x <= last; }
};

inline Range range(int first, int last) {
    return {first, last};
}

inline bool matchSingle(int x, const Range& arg) {
    return arg.contains(x);
}

inline bool matchSingle(int x, int y) {
    return x == y;
}

template <typename... T>
bool matchAny(int value, T... arg) {
    return (matchSingle(value, arg) || ...);
}

}  // namespace metkit::mars2grib::util::param_matcher
