/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/TestCaseLabels.h"

#include <set>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Tokenizer.h"

#include "metkit/mars2grib/api/Mars2GribClassify.h"
#include "metkit/mars2grib/testing-utils/TestRecord.h"

namespace metkit::mars2grib::testing_utils::run_tests {

namespace {

constexpr const char* timeShapeKey = "timeShape";

std::string marsLabel(const eckit::LocalConfiguration& mars, const std::string& key) {
    if (!mars.has(key)) {
        return missingLabel;
    }
    if (mars.isString(key)) {
        const std::string value = mars.getString(key);
        return value.empty() ? missingLabel : value;
    }
    if (mars.isIntegral(key)) {
        return std::to_string(mars.getLong(key));
    }
    return missingLabel;
}

std::string timeShapeLabel(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                           const eckit::LocalConfiguration& opt) {
    try {
        metkit::mars2grib::Mars2GribClassify classifier{opt};
        const auto spec                      = classifier.computeProductTimeSpec(mars, misc);
        const eckit::LocalConfiguration json = parseRecord(spec.json, RecordParser::Json);
        if (json.has("shapeType") && json.isString("shapeType")) {
            return json.getString("shapeType");
        }
    }
    catch (...) {
    }
    return missingLabel;
}

}  // namespace

TestCaseLabeler::TestCaseLabeler(const std::string& keys) {
    std::vector<std::string> names;
    eckit::Tokenizer(",")(keys, names);
    if (names.empty()) {
        throw eckit::UserError("At least one block key is needed", Here());
    }
    std::set<std::string> seen;
    for (const auto& name : names) {
        if (!seen.insert(name).second) {
            throw eckit::UserError("Duplicate block key `" + name + "`", Here());
        }
        keys_.push_back(eckit::blockfile::LabelKey{name, eckit::blockfile::LabelType::String});
        derived_ = derived_ || name == timeShapeKey;
    }
    eckit::blockfile::validateKeys(keys_);
}

eckit::blockfile::Labels TestCaseLabeler::labels(const eckit::LocalConfiguration& mars,
                                                 const eckit::LocalConfiguration& misc,
                                                 const eckit::LocalConfiguration& opt) const {
    eckit::blockfile::Labels labels;
    labels.reserve(keys_.size());
    for (const auto& key : keys_) {
        labels.emplace_back(key.name == timeShapeKey ? timeShapeLabel(mars, misc, opt) : marsLabel(mars, key.name));
    }
    return labels;
}

std::uint64_t fnv1a(std::string_view text, std::uint64_t hash) {
    for (const char c : text) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::uint64_t testCaseIdentity(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                               const eckit::LocalConfiguration& opt) {
    // eckit serialises objects with sorted keys, so equal inputs give equal hashes.
    std::uint64_t hash = fnv1a(configurationToJson(mars));
    hash               = fnv1a("\x1F", hash);
    hash               = fnv1a(configurationToJson(misc), hash);
    hash               = fnv1a("\x1F", hash);
    return fnv1a(configurationToJson(opt), hash);
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
