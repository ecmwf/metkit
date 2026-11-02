/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Plan.h
/// @brief Simplification actions and plans of the test-case stripping pipeline.
///
/// A plan is an ordered list of actions; a test is kept if every action keeps it. Actions keep their
/// own state (what they already let through), so the decision for a record can depend on the records
/// seen before it. In the MPI tool every record is processed on the rank owning its routing key
/// (class, stream, type, levtype, time representation), which is therefore implicitly part of every
/// action's group.
///
/// Actions (YAML, `type` selects the action):
///   select         include: <rules>   and/or   exclude: <rules>         (MarsRecordFilter syntax)
///   reduce         match: <rules> (optional), vary: [keys], keep: N,
///                  group-by: [mars | misc | opt | time-representation | mars.<key> | misc.<key> | opt.<key>]
///                  per group keep the first N distinct combinations of the `vary` MARS keys;
///                  records not matching `match` pass unchanged
///   first-emission keep only the first emission of each time representation (step == outer time range)
///   deduplicate    drop repeated (mars, misc, opt)

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/testing-utils/strip/Envelope.h"

namespace metkit::mars2grib::testing_utils::strip {

class Action {
public:

    explicit Action(std::string description) : description_{std::move(description)} {}
    virtual ~Action() = default;

    Action(const Action&)            = delete;
    Action& operator=(const Action&) = delete;

    /// Decide and count.
    bool apply(const TestView& test) {
        const bool keep = decide(test);
        ++(keep ? kept_ : dropped_);
        return keep;
    }

    const std::string& description() const { return description_; }
    std::uint64_t kept() const { return kept_; }
    std::uint64_t dropped() const { return dropped_; }

private:

    virtual bool decide(const TestView& test) = 0;

    std::string description_;
    std::uint64_t kept_{0};
    std::uint64_t dropped_{0};
};

/// Create an action from its YAML configuration (throws eckit::UserError on invalid configuration).
std::unique_ptr<Action> makeAction(const eckit::LocalConfiguration& config);

class Plan {
public:

    /// From a list of action configurations.
    explicit Plan(const std::vector<eckit::LocalConfiguration>& actions);

    /// True if every action keeps the test (stops at the first action that drops it).
    bool keep(const TestView& test);

    const std::vector<std::unique_ptr<Action>>& actions() const { return actions_; }

private:

    std::vector<std::unique_ptr<Action>> actions_;
};

}  // namespace metkit::mars2grib::testing_utils::strip
