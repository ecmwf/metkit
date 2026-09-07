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

        const auto set        = requireObject(operation, "set");
        const std::string key = requireString(set, "key");
        const auto datatype   = requireObject(set, "datatype");
        const std::string type = requireString(datatype, "type");
        const long rank         = requireLong(datatype, "rank");
        const long size         = requireLong(datatype, "size");

        if (size < 0 || (rank == 0 && size != 1)) {
            throw eckit::UserError("Invalid RecordingDictionary datatype size", Here());
        }

        if (rank == 0 && type == "boolean" && set.isBoolean("value")) {
            record_set(key, set.getBool("value"));
        }
        else if (rank == 0 && type == "integer" && set.isIntegral("value")) {
            record_set(key, set.getLong("value"));
        }
        else if (rank == 0 && type == "double" &&
                 (set.isFloatingPoint("value") || set.isIntegral("value"))) {
            record_set(key, set.isFloatingPoint("value") ? set.getDouble("value")
                                                          : static_cast<double>(set.getLong("value")));
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
            if (!summary.has("average") ||
                !(summary.isFloatingPoint("average") || summary.isIntegral("average")) || size < 0) {
                throw eckit::UserError("Invalid RecordingDictionary values summary", Here());
            }
            const double average = summary.isFloatingPoint("average") ? summary.getDouble("average")
                                                                        : static_cast<double>(summary.getLong("average"));
            operations_.emplace_back(OperationKind::Set, key,
                                     std::make_unique<TypedRecordedValue<ValuesSummary>>(
                                         ValuesSummary{static_cast<std::size_t>(size), average}));
        }
        else if (rank == 1 && type == "double" &&
                 (set.isFloatingPointList("value") || set.isIntegralList("value"))) {
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

}  // namespace metkit::mars2grib::testing_utils
