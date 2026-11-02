/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"

#include <cstdint>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "metkit/datamod/Dict.h"

namespace metkit::mars2grib::testing_utils {
namespace {

eckit::LocalConfiguration requireObject(const eckit::LocalConfiguration& configuration, const std::string& key) {
    if (!configuration.has(key) || !configuration.isSubConfiguration(key)) {
        throw eckit::UserError("RecordingDictionary operation requires object `" + key + "`", Here());
    }
    return configuration.getSubConfiguration(key);
}

std::string requireString(const eckit::LocalConfiguration& configuration, const std::string& key) {
    if (!configuration.has(key) || !configuration.isString(key)) {
        throw eckit::UserError("RecordingDictionary operation requires string `" + key + "`", Here());
    }
    return configuration.getString(key);
}

long requireLong(const eckit::LocalConfiguration& configuration, const std::string& key) {
    if (!configuration.has(key) || !configuration.isIntegral(key)) {
        throw eckit::UserError("RecordingDictionary operation requires integer `" + key + "`", Here());
    }
    return configuration.getLong(key);
}


using metkit::datamod::Dict;
using metkit::datamod::Value;

bool isEmptyList(const Value& v) {
    return (v.isIntList() || v.isDoubleList() || v.isStringList() || v.isDictList()) && v.listSize() == 0;
}

const Dict& requireObject(const Dict& dict, const std::string& key) {
    const Value* v = dict.find(key);
    if (v == nullptr || !v->isDict()) {
        throw eckit::UserError("RecordingDictionary operation requires object `" + key + "`", Here());
    }
    return v->asDict();
}

std::string requireString(const Dict& dict, const std::string& key) {
    const Value* v = dict.find(key);
    if (v == nullptr || !v->isString()) {
        throw eckit::UserError("RecordingDictionary operation requires string `" + key + "`", Here());
    }
    return v->asString();
}

long requireLong(const Dict& dict, const std::string& key) {
    const Value* v = dict.find(key);
    if (v == nullptr || !v->isInt()) {
        throw eckit::UserError("RecordingDictionary operation requires integer `" + key + "`", Here());
    }
    return static_cast<long>(v->asInt());
}

double asNumber(const Value& v) {
    return v.isInt() ? static_cast<double>(v.asInt()) : v.asDouble();
}

std::vector<double> asNumberList(const Value& v) {
    if (v.isDoubleList()) {
        return v.asDoubleList();
    }
    if (v.isIntList()) {
        return std::vector<double>(v.asIntList().begin(), v.asIntList().end());
    }
    return {};
}

}  // namespace

RecordingDictionary::RecordingDictionary(const eckit::LocalConfiguration& configuration) {
    if (!configuration.has("operations") || !configuration.isSubConfigurationList("operations")) {
        throw eckit::UserError("RecordingDictionary requires array `operations`", Here());
    }

    for (const auto& operation : configuration.getSubConfigurations("operations")) {
        if (operation.has("make_from_sample")) {
            if (operation.keys().size() != 1) {
                throw eckit::UserError("RecordingDictionary operation must contain exactly one operation", Here());
            }
            const auto value = requireObject(operation, "make_from_sample");
            record_make_from_sample(requireString(value, "sample"));
            continue;
        }

        if (operation.has("clone")) {
            if (operation.keys().size() != 1) {
                throw eckit::UserError("RecordingDictionary operation must contain exactly one operation", Here());
            }
            const auto value = requireObject(operation, "clone");
            const long count = requireLong(value, "source_operation_count");
            if (count < 0) {
                throw eckit::UserError("RecordingDictionary clone count must not be negative", Here());
            }
            record_clone(static_cast<std::size_t>(count));
            continue;
        }

        if (operation.has("set_missing")) {
            if (operation.keys().size() != 1) {
                throw eckit::UserError("RecordingDictionary operation must contain exactly one operation", Here());
            }
            const auto value = requireObject(operation, "set_missing");
            record_set_missing(requireString(value, "key"));
            continue;
        }

        if (!operation.has("set")) {
            throw eckit::UserError("Unknown RecordingDictionary operation", Here());
        }
        if (operation.keys().size() != 1) {
            throw eckit::UserError("RecordingDictionary operation must contain exactly one operation", Here());
        }

        const auto set         = requireObject(operation, "set");
        const std::string key  = requireString(set, "key");
        const auto datatype    = requireObject(set, "datatype");
        const std::string type = requireString(datatype, "type");
        const long rank        = requireLong(datatype, "rank");
        const long size        = requireLong(datatype, "size");

        if (size < 0 || (rank == 0 && size != 1)) {
            throw eckit::UserError("Invalid RecordingDictionary datatype size", Here());
        }

        if (rank == 0 && type == "boolean" && set.isBoolean("value")) {
            record_set(key, set.getBool("value"));
        }
        else if (rank == 0 && type == "integer" && set.isIntegral("value")) {
            record_set(key, set.getLong("value"));
        }
        else if (rank == 0 && type == "double" && (set.isFloatingPoint("value") || set.isIntegral("value"))) {
            record_set(
                key, set.isFloatingPoint("value") ? set.getDouble("value") : static_cast<double>(set.getLong("value")));
        }
        else if (rank == 0 && type == "string" && set.isString("value")) {
            record_set(key, set.getString("value"));
        }
        else if (rank == 1 && type == "integer" && set.isIntegralList("value")) {
            const auto values = set.getLongVector("value");
            if (values.size() != static_cast<std::size_t>(size)) {
                throw eckit::UserError("RecordingDictionary vector size does not match datatype", Here());
            }
            record_set(key, values);
        }
        else if (rank == 1 && type == "double" && key == "values" && set.isSubConfiguration("value")) {
            const auto summary = set.getSubConfiguration("value");
            if (!summary.has("average") || !(summary.isFloatingPoint("average") || summary.isIntegral("average")) ||
                size < 0) {
                throw eckit::UserError("Invalid RecordingDictionary values summary", Here());
            }
            const double average = summary.isFloatingPoint("average") ? summary.getDouble("average")
                                                                      : static_cast<double>(summary.getLong("average"));
            operations_.emplace_back(OperationKind::Set, key,
                                     std::make_unique<TypedRecordedValue<ValuesSummary>>(
                                         ValuesSummary{static_cast<std::size_t>(size), average}));
        }
        else if (rank == 1 && type == "double" && (set.isFloatingPointList("value") || set.isIntegralList("value"))) {
            const auto values = set.getDoubleVector("value");
            if (values.size() != static_cast<std::size_t>(size)) {
                throw eckit::UserError("RecordingDictionary vector size does not match datatype", Here());
            }
            record_set(key, values);
        }
        else if (rank == 1 && type == "string" && set.isStringList("value")) {
            const auto values = set.getStringVector("value");
            if (values.size() != static_cast<std::size_t>(size)) {
                throw eckit::UserError("RecordingDictionary vector size does not match datatype", Here());
            }
            record_set(key, values);
        }
        else if (rank == 1 && type == "byte" && set.isIntegralList("value")) {
            std::vector<uint8_t> bytes;
            for (const long item : set.getLongVector("value")) {
                if (item < 0 || item > 255) {
                    throw eckit::UserError("RecordingDictionary byte value is outside 0..255", Here());
                }
                bytes.push_back(static_cast<uint8_t>(item));
            }
            if (bytes.size() != static_cast<std::size_t>(size)) {
                throw eckit::UserError("RecordingDictionary vector size does not match datatype", Here());
            }
            record_set(key, std::move(bytes));
        }
        else {
            throw eckit::UserError("Unsupported RecordingDictionary datatype `" + type + "`", Here());
        }
    }
}


RecordingDictionary::RecordingDictionary(const metkit::datamod::Dict& configuration) {
    const Value* operations = configuration.find("operations");
    if (operations == nullptr || !(operations->isDictList() || isEmptyList(*operations))) {
        throw eckit::UserError("RecordingDictionary requires array `operations`", Here());
    }
    if (!operations->isDictList()) {
        return;
    }

    for (const Dict& operation : operations->asDictList()) {
        if (operation.size() != 1) {
            throw eckit::UserError("RecordingDictionary operation must contain exactly one operation", Here());
        }

        if (operation.has("make_from_sample")) {
            record_make_from_sample(requireString(requireObject(operation, "make_from_sample"), "sample"));
            continue;
        }

        if (operation.has("clone")) {
            const long count = requireLong(requireObject(operation, "clone"), "source_operation_count");
            if (count < 0) {
                throw eckit::UserError("RecordingDictionary clone count must not be negative", Here());
            }
            record_clone(static_cast<std::size_t>(count));
            continue;
        }

        if (operation.has("set_missing")) {
            record_set_missing(requireString(requireObject(operation, "set_missing"), "key"));
            continue;
        }

        if (!operation.has("set")) {
            throw eckit::UserError("Unknown RecordingDictionary operation", Here());
        }

        const Dict& set        = requireObject(operation, "set");
        const std::string key  = requireString(set, "key");
        const Dict& datatype   = requireObject(set, "datatype");
        const std::string type = requireString(datatype, "type");
        const long rank        = requireLong(datatype, "rank");
        const long size        = requireLong(datatype, "size");

        if (size < 0 || (rank == 0 && size != 1)) {
            throw eckit::UserError("Invalid RecordingDictionary datatype size", Here());
        }

        static const Value null;
        const Value* found = set.find("value");
        const Value& value = found == nullptr ? null : *found;

        const auto checkSize = [&](std::size_t actual) {
            if (actual != static_cast<std::size_t>(size)) {
                throw eckit::UserError("RecordingDictionary vector size does not match datatype", Here());
            }
        };

        if (rank == 0 && type == "boolean" && value.isBool()) {
            record_set(key, value.asBool());
        }
        else if (rank == 0 && type == "integer" && value.isInt()) {
            record_set(key, static_cast<long long>(value.asInt()));
        }
        else if (rank == 0 && type == "double" && (value.isDouble() || value.isInt())) {
            record_set(key, asNumber(value));
        }
        else if (rank == 0 && type == "string" && value.isString()) {
            record_set(key, value.asString());
        }
        else if (rank == 1 && type == "integer" && (value.isIntList() || isEmptyList(value))) {
            std::vector<long> values;
            if (value.isIntList()) {
                values.assign(value.asIntList().begin(), value.asIntList().end());
            }
            checkSize(values.size());
            record_set(key, values);
        }
        else if (rank == 1 && type == "double" && key == "values" && value.isDict()) {
            const Value* average = value.asDict().find("average");
            if (average == nullptr || !(average->isDouble() || average->isInt())) {
                throw eckit::UserError("Invalid RecordingDictionary values summary", Here());
            }
            operations_.emplace_back(OperationKind::Set, key,
                                     std::make_unique<TypedRecordedValue<ValuesSummary>>(
                                         ValuesSummary{static_cast<std::size_t>(size), asNumber(*average)}));
        }
        else if (rank == 1 && type == "double" && (value.isDoubleList() || value.isIntList() || isEmptyList(value))) {
            const std::vector<double> values = asNumberList(value);
            checkSize(values.size());
            record_set(key, values);
        }
        else if (rank == 1 && type == "string" && (value.isStringList() || isEmptyList(value))) {
            const std::vector<std::string> values =
                value.isStringList() ? value.asStringList() : std::vector<std::string>{};
            checkSize(values.size());
            record_set(key, values);
        }
        else if (rank == 1 && type == "byte" && (value.isIntList() || isEmptyList(value))) {
            std::vector<uint8_t> bytes;
            if (value.isIntList()) {
                for (const std::int64_t item : value.asIntList()) {
                    if (item < 0 || item > 255) {
                        throw eckit::UserError("RecordingDictionary byte value is outside 0..255", Here());
                    }
                    bytes.push_back(static_cast<uint8_t>(item));
                }
            }
            checkSize(bytes.size());
            record_set(key, std::move(bytes));
        }
        else {
            throw eckit::UserError("Unsupported RecordingDictionary datatype `" + type + "`", Here());
        }
    }
}

}  // namespace metkit::mars2grib::testing_utils
