/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::run_tests {

/// Rewrites of the `mars` input of test cases, to correct wrong requests before regeneration.
///
/// Loaded from a YAML file:
///
///     overrides:
///       - match: type=gbf/gwt,param=228228   # --exclude syntax; rules separated by ';'
///         when: window-from-start              # optional, see below
///         set: {timespan: fs}                  # optional: keys to add or replace
///         remove: [foo]                        # optional: keys to remove
///
/// `when` conditions:
///  - window-from-start: `timespan` is a window of whole hours (`<N>h` or `<N>`) equal to `step`,
///    i.e. the window starts at step 0.
///
/// Every matching rule is applied, in file order, each one seeing the result of the previous ones.
class MarsOverrides {
public:

    MarsOverrides() = default;

    static MarsOverrides fromFile(const std::string& path);

    bool empty() const { return rules_.empty(); }
    std::size_t numberOfRules() const { return rules_.size(); }

    /// Short description of a rule, for reports.
    const std::string& description(std::size_t rule) const { return rules_[rule].description; }

    /// Applies the matching rules to `mars`; `applied[i]` is incremented for every rule i that changed it.
    /// `applied` must have numberOfRules() entries.
    void apply(eckit::LocalConfiguration& mars, std::vector<std::size_t>& applied) const;

private:

    enum class When {
        Always,
        WindowFromStart,
    };

    struct Rule {
        std::string description;
        MarsRecordFilter match;
        When when{When::Always};
        eckit::LocalConfiguration set;
        std::vector<std::string> remove;
    };

    std::vector<Rule> rules_;
};

}  // namespace metkit::mars2grib::testing_utils::run_tests
