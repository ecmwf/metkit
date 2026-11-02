/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/strip/Plan.h"

#include <algorithm>
#include <optional>
#include <sstream>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "eckit/exception/Exceptions.h"

#include "metkit/datamod/Json.h"
#include "metkit/mars2grib/testing-utils/TestCaseLabels.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::strip {

namespace {

using metkit::datamod::Dict;
using metkit::datamod::Value;
using run_tests::fnv1a;
using run_tests::MarsRecordFilter;

std::uint64_t mix(std::uint64_t hash, std::string_view text) {
    return fnv1a("\x1F", fnv1a(text, hash));
}

/// Hash of a dictionary independent of key order (keys sorted, nested dictionaries recursively).
std::uint64_t canonicalHash(const Dict& dict, std::uint64_t hash, const std::vector<std::string>& skip = {}) {
    std::vector<const Dict::Entry*> entries;
    for (const auto& entry : dict.entries()) {
        if (std::find(skip.begin(), skip.end(), entry.key) == skip.end()) {
            entries.push_back(&entry);
        }
    }
    std::sort(entries.begin(), entries.end(), [](const auto* a, const auto* b) { return a->key < b->key; });
    for (const auto* entry : entries) {
        hash = mix(hash, entry->key);
        hash = entry->value.isDict() ? canonicalHash(entry->value.asDict(), mix(hash, "{"))
                                     : mix(hash, metkit::datamod::toJson(entry->value));
    }
    return mix(hash, "}");
}

const Dict& sectionOf(const TestView& test, const std::string& name) {
    if (name == "mars") {
        return test.mars;
    }
    if (name == "misc") {
        return test.misc;
    }
    if (name == "opt") {
        return test.opt;
    }
    throw eckit::UserError("Unknown section `" + name + "` (mars, misc, opt)", Here());
}

std::vector<std::string> stringList(const eckit::LocalConfiguration& config, const std::string& key) {
    if (!config.has(key)) {
        return {};
    }
    if (config.isString(key)) {
        return {config.getString(key)};
    }
    if (config.isStringList(key)) {
        return config.getStringVector(key);
    }
    throw eckit::UserError("`" + key + "` must be a string or a list of strings", Here());
}

std::optional<MarsRecordFilter> rules(const eckit::LocalConfiguration& config, const std::string& key) {
    if (!config.has(key)) {
        return std::nullopt;
    }
    std::string text;
    for (const auto& rule : stringList(config, key)) {
        text += rule + ";";
    }
    MarsRecordFilter filter;
    filter.addRules(text);
    return filter;
}

std::string describe(const eckit::LocalConfiguration& config) {
    std::ostringstream os;
    os << config;
    return os.str();
}

//----------------------------------------------------------------------------------------------------------------------

class SelectAction final : public Action {
public:

    explicit SelectAction(const eckit::LocalConfiguration& config) :
        Action{"select " + describe(config)}, include_{rules(config, "include")}, exclude_{rules(config, "exclude")} {
        if (!include_ && !exclude_) {
            throw eckit::UserError("select needs `include` and/or `exclude`", Here());
        }
    }

private:

    bool decide(const TestView& test) override {
        if (include_ && !include_->matches(test.mars)) {
            return false;
        }
        return !(exclude_ && exclude_->matches(test.mars));
    }

    std::optional<MarsRecordFilter> include_;
    std::optional<MarsRecordFilter> exclude_;
};

class ReduceAction final : public Action {
public:

    explicit ReduceAction(const eckit::LocalConfiguration& config) :
        Action{"reduce " + describe(config)},
        match_{rules(config, "match")},
        vary_{stringList(config, "vary")},
        groupBy_{stringList(config, "group-by")},
        keep_{config.has("keep") ? config.getLong("keep") : 1} {
        if (vary_.empty()) {
            throw eckit::UserError("reduce needs `vary`", Here());
        }
        if (keep_ < 1) {
            throw eckit::UserError("reduce `keep` must be at least 1", Here());
        }
        if (groupBy_.empty()) {
            groupBy_ = {"mars"};
        }
        for (const auto& item : groupBy_) {
            const bool valid = item == "mars" || item == "misc" || item == "opt" || item == "time-representation" ||
                               item.rfind("mars.", 0) == 0 || item.rfind("misc.", 0) == 0 || item.rfind("opt.", 0) == 0;
            if (!valid) {
                throw eckit::UserError("Invalid reduce group-by `" + item + "`", Here());
            }
        }
    }

private:

    bool decide(const TestView& test) override {
        if (match_ && !match_->matches(test.mars)) {
            return true;
        }

        // The routing key is always part of the group.
        std::uint64_t group = mix(fnv1a("group"), std::to_string(test.envelope.route));
        for (const auto& item : groupBy_) {
            group = mix(group, item);
            if (item == "time-representation") {
                group = mix(group, test.envelope.timeRepresentation());
            }
            else if (item == "mars" || item == "misc" || item == "opt") {
                group =
                    canonicalHash(sectionOf(test, item), group, item == "mars" ? vary_ : std::vector<std::string>{});
            }
            else {
                const auto dot      = item.find('.');
                const Dict& section = sectionOf(test, item.substr(0, dot));
                const Value* value  = section.find(item.substr(dot + 1));
                group               = mix(group, value == nullptr ? missingValue : metkit::datamod::toJson(*value));
            }
        }

        std::uint64_t combination = fnv1a("vary");
        for (const auto& key : vary_) {
            const Value* value = test.mars.find(key);
            combination        = mix(combination, value == nullptr ? missingValue : metkit::datamod::toJson(*value));
        }

        auto& seen = seen_[group];
        if (std::find(seen.begin(), seen.end(), combination) != seen.end()) {
            return true;  // a combination already kept for this group (e.g. differs only in misc/opt)
        }
        if (static_cast<long>(seen.size()) < keep_) {
            seen.push_back(combination);
            return true;
        }
        return false;
    }

    std::optional<MarsRecordFilter> match_;
    std::vector<std::string> vary_;
    std::vector<std::string> groupBy_;
    long keep_;
    std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> seen_;
};

class FirstEmissionAction final : public Action {
public:

    explicit FirstEmissionAction(const eckit::LocalConfiguration& config) :
        Action{"first-emission " + describe(config)} {}

private:

    bool decide(const TestView& test) override { return test.envelope.firstEmission; }
};

class DeduplicateAction final : public Action {
public:

    explicit DeduplicateAction(const eckit::LocalConfiguration& config) : Action{"deduplicate " + describe(config)} {}

private:

    bool decide(const TestView& test) override {
        std::uint64_t identity = canonicalHash(test.mars, fnv1a("mars"));
        identity               = canonicalHash(test.misc, mix(identity, "misc"));
        identity               = canonicalHash(test.opt, mix(identity, "opt"));
        return seen_.insert(identity).second;
    }

    std::unordered_set<std::uint64_t> seen_;
};

}  // namespace

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<Action> makeAction(const eckit::LocalConfiguration& config) {
    if (!config.has("type") || !config.isString("type")) {
        throw eckit::UserError("Every action needs a `type`", Here());
    }
    const std::string type = config.getString("type");
    if (type == "select") {
        return std::make_unique<SelectAction>(config);
    }
    if (type == "reduce") {
        return std::make_unique<ReduceAction>(config);
    }
    if (type == "first-emission") {
        return std::make_unique<FirstEmissionAction>(config);
    }
    if (type == "deduplicate") {
        return std::make_unique<DeduplicateAction>(config);
    }
    throw eckit::UserError("Unknown action type `" + type + "` (select, reduce, first-emission, deduplicate)", Here());
}

Plan::Plan(const std::vector<eckit::LocalConfiguration>& actions) {
    for (const auto& config : actions) {
        actions_.push_back(makeAction(config));
    }
}

bool Plan::keep(const TestView& test) {
    for (auto& action : actions_) {
        if (!action->apply(test)) {
            return false;
        }
    }
    return true;
}

}  // namespace metkit::mars2grib::testing_utils::strip
