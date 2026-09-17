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

#include <tuple>
#include <utility>

#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::util::param_matcher {

struct Range {
    int first;
    int last;
    Range(int first_, int last_) : first(first_), last(last_) {}
    bool contains(int x) const { return x >= first && x <= last; }
};

using range = Range;

template <class Cntx_t>
inline bool matchSingle(int x, const Range& arg, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const bool result = arg.contains(x);
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <class Cntx_t>
inline bool matchSingle(int x, int y, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const bool result = x == y;
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <typename Tuple, class Cntx_t, std::size_t... I>
bool matchAnyImpl(int value, Tuple& args, std::index_sequence<I...>, Cntx_t& cntx) {
    utils::profiling::profileEnterFunction(cntx, Here());
    const bool result =
        (matchSingle(value, std::get<I>(args), utils::profiling::callSite(cntx, Here())) || ...);
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

template <typename... Args>
bool matchAny(int value, Args&&... args) {
    static_assert(sizeof...(Args) >= 2, "matchAny requires at least one matcher and a context");
    auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);
    auto& cntx = std::get<sizeof...(Args) - 1>(tuple);
    utils::profiling::profileEnterFunction(cntx, Here());
    const bool result = matchAnyImpl(value, tuple, std::make_index_sequence<sizeof...(Args) - 1>{},
                                     utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::util::param_matcher
