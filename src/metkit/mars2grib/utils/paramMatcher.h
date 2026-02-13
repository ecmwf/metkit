/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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

}
