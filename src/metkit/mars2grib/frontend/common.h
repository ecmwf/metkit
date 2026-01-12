/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include <type_traits>
#include "eckit/config/LocalConfiguration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

using metkit::mars2grib::utils::dict_traits::get_opt;
using metkit::mars2grib::utils::dict_traits::has;
using metkit::mars2grib::utils::dict_traits::set_or_throw;

namespace metkit::mars2grib::frontend {

//============================ Recursive Setters =============================//

template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, const char*>, int> = 0>
void setRecursive(eckit::LocalConfiguration&, const std::string&, T, bool);

template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, const char*>, int> = 0>
void setRecursive(eckit::LocalConfiguration&, const std::string&, T);

inline void setRecursive(eckit::LocalConfiguration& config, const std::string& key, const char* value,
                         bool ignoreIfAlreadySet) {
    setRecursive(config, key, std::string{value}, ignoreIfAlreadySet);
}

inline void setRecursive(eckit::LocalConfiguration& config, const std::string& key, const char* value) {
    setRecursive(config, key, std::string{value}, false);
}

template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, const char*>, int>>
void setRecursive(eckit::LocalConfiguration& config, const std::string& key, T value) {
    setRecursive(config, key, value, false);
}

template <typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, const char*>, int>>
void setRecursive(eckit::LocalConfiguration& config, const std::string& key, T value, bool ignoreIfAlreadySet) {
    const auto pos = key.find('.');
    if (pos == std::string::npos) {
        if (!ignoreIfAlreadySet || !has(config, key)) {
            set_or_throw<T>(config, key, value);
        }
    }
    else {
        auto first = key.substr(0, pos);
        auto rest  = key.substr(pos + 1);

        auto subConfig = get_opt<eckit::LocalConfiguration>(config, first).value_or(eckit::LocalConfiguration{});
        setRecursive(subConfig, rest, value);
        set_or_throw<eckit::LocalConfiguration>(config, first, subConfig);
    }
}

inline void setRecursiveDefault(eckit::LocalConfiguration& config, const std::string& key, const std::string& value) {
    setRecursive(config, key, value, true);
}

//================================= Matchers =================================//

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

//============================= Special Setters ==============================//

// TODO : Replace all calls to setPDT?
inline void setPDT(eckit::LocalConfiguration& sections, const std::string& key, const std::string& value) {
    setRecursive(sections, "product-definition-section.product-categories." + key, value);
}

inline void setPointInTime(eckit::LocalConfiguration& sections) {
    setPDT(sections, "timeExtent", "pointInTime");
    setRecursiveDefault(sections, "product-definition-section.point-in-time-configurator.type", "default");
}

inline void setSinceLastPostProcessingStep(eckit::LocalConfiguration& sections) {
    setPDT(sections, "timeExtent", "timeRange");
    setRecursiveDefault(sections, "product-definition-section.time-statistics-configurator.type",
                        "since-last-post-processing-step");
}

inline void setFixedTimeRange(eckit::LocalConfiguration& sections, const std::string& length) {
    setPDT(sections, "timeExtent", "timeRange");
    setRecursive(sections, "product-definition-section.time-statistics-configurator.type", "fixed-timerange");
    setRecursive(sections, "product-definition-section.time-statistics-configurator.overall-length-of-timerange",
                 length);
}

inline void setTypeOfStatisticalProcessing(eckit::LocalConfiguration& sections,
                                           const std::string& typeOfStatisticalProcessing) {
    setRecursive(sections, "product-definition-section.time-statistics-configurator.type-of-statistical-processing",
                 typeOfStatisticalProcessing);
}

inline void setTypeOfLevel(eckit::LocalConfiguration& sections, const std::string& type) {
    setRecursive(sections, "product-definition-section.level-configurator.type", type);
}

inline void setFixedLevel(eckit::LocalConfiguration& sections, const long level) {
    // TODO : Make sure level-configurator.type is set?
    setRecursive(sections, "product-definition-section.level-configurator.fixed-level", level);
}

//============================================================================//

}  // namespace metkit::mars2grib::frontend
