/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/MarsOverrides.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <optional>

#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"

namespace metkit::mars2grib::testing_utils::run_tests {
namespace {

/// Copies `key` of `from` into `to`; false for values that are not scalars or lists of scalars.
bool copyValue(const eckit::LocalConfiguration& from, const std::string& key, eckit::LocalConfiguration& to) {
    if (from.isString(key)) {
        to.set(key, from.getString(key));
    }
    else if (from.isBoolean(key)) {
        to.set(key, from.getBool(key));
    }
    else if (from.isIntegral(key)) {
        to.set(key, from.getLong(key));
    }
    else if (from.isFloatingPoint(key)) {
        to.set(key, from.getDouble(key));
    }
    else if (from.isStringList(key)) {
        to.set(key, from.getStringVector(key));
    }
    else if (from.isIntegralList(key) || from.isFloatingPointList(key)) {
        // eckit classifies a list by its first element only: read as doubles, keep integers if all are whole.
        const std::vector<double> values = from.getDoubleVector(key);
        if (std::all_of(values.begin(), values.end(), [](double v) { return std::floor(v) == v; })) {
            to.set(key, std::vector<long>(values.begin(), values.end()));
        }
        else {
            to.set(key, values);
        }
    }
    else {
        return false;
    }
    return true;
}

/// `key` as a whole number of hours: an integer, or a string `<N>` or `<N>h`.
std::optional<long> wholeHours(const eckit::LocalConfiguration& mars, const std::string& key) {
    if (!mars.has(key)) {
        return std::nullopt;
    }
    if (mars.isIntegral(key)) {
        return mars.getLong(key);
    }
    if (!mars.isString(key)) {
        return std::nullopt;
    }
    std::string text = mars.getString(key);
    if (!text.empty() && (text.back() == 'h' || text.back() == 'H')) {
        text.pop_back();
    }
    if (text.empty() || !std::all_of(text.begin(), text.end(), [](unsigned char c) { return std::isdigit(c); })) {
        return std::nullopt;
    }
    return std::stol(text);
}

}  // namespace

MarsOverrides MarsOverrides::fromFile(const std::string& path) {
    const eckit::PathName file{path};
    if (!file.exists()) {
        throw eckit::UserError("Overrides file `" + path + "` does not exist", Here());
    }
    const eckit::YAMLConfiguration config{file};
    if (!config.has("overrides")) {
        throw eckit::UserError("Overrides file `" + path + "` has no `overrides` list", Here());
    }

    MarsOverrides overrides;
    std::size_t index = 0;
    for (const auto& item : config.getSubConfigurations("overrides")) {
        const std::string where = "overrides[" + std::to_string(index++) + "] in `" + path + "`";
        for (const auto& key : item.keys()) {
            if (key != "match" && key != "when" && key != "set" && key != "remove") {
                throw eckit::UserError("Unknown key `" + key + "` in " + where, Here());
            }
        }

        Rule rule;
        if (!item.isString("match") || item.getString("match").empty()) {
            throw eckit::UserError("Missing `match` in " + where, Here());
        }
        rule.match.addRules(item.getString("match"));
        rule.description = item.getString("match");

        if (item.has("when")) {
            const std::string when = item.getString("when");
            if (when != "window-from-start") {
                throw eckit::UserError("Unknown `when: " + when + "` in " + where + " (known: window-from-start)",
                                       Here());
            }
            rule.when = When::WindowFromStart;
            rule.description += " when " + when;
        }
        if (item.has("set")) {
            const eckit::LocalConfiguration set = item.getSubConfiguration("set");
            for (const auto& key : set.keys()) {
                if (!copyValue(set, key, rule.set)) {
                    throw eckit::UserError("Unsupported value for `set." + key + "` in " + where, Here());
                }
                rule.description += (key == set.keys().front() ? " -> set " : ",") + key;
            }
        }
        if (item.has("remove")) {
            rule.remove = item.getStringVector("remove");
            rule.description += " -> remove";
            for (const auto& key : rule.remove) {
                rule.description += " " + key;
            }
        }
        if (rule.set.keys().empty() && rule.remove.empty()) {
            throw eckit::UserError("Nothing to `set` or `remove` in " + where, Here());
        }
        overrides.rules_.push_back(std::move(rule));
    }
    return overrides;
}

void MarsOverrides::apply(eckit::LocalConfiguration& mars, std::vector<std::size_t>& applied) const {
    for (std::size_t i = 0; i < rules_.size(); ++i) {
        const Rule& rule = rules_[i];
        if (!rule.match.matches(mars)) {
            continue;
        }
        if (rule.when == When::WindowFromStart) {
            const auto timespan = wholeHours(mars, "timespan");
            const auto step     = wholeHours(mars, "step");
            if (!timespan || !step || *timespan <= 0 || *timespan != *step) {
                continue;
            }
        }
        for (const auto& key : rule.set.keys()) {
            copyValue(rule.set, key, mars);
        }
        for (const auto& key : rule.remove) {
            if (mars.has(key)) {
                mars.remove(key);
            }
        }
        ++applied[i];
    }
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
