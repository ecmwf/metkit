/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file readOptionsFromInitializerList.h
/// @brief Parse Mars2Grib Options from a compact initializer list.
///
/// This file is intended to be included into `Mars2Grib.cc` so the parser can
/// remain implementation-only while still being the single source of truth.
///

#include <string_view>
#include <unordered_set>

#include "eckit/value/Value.h"

#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::detail {

namespace exceptions = metkit::mars2grib::utils::exceptions;

[[noreturn]] inline void throwInvalidOptionType(std::string_view key, const eckit::Value& value,
                                                std::string_view expected) {
    throw exceptions::Mars2GribDictException("Option `" + std::string(key) + "` has value type `" + value.typeName() +
                                                 "`; expected " + std::string(expected),
                                             Here());
}

inline bool readBool(std::string_view key, const eckit::Value& value) {
    if (!value.isBool()) {
        throwInvalidOptionType(key, value, "bool");
    }

    return value.as<bool>();
}

inline std::string readString(std::string_view key, const eckit::Value& value) {
    if (!value.isString()) {
        throwInvalidOptionType(key, value, "string");
    }

    return value.as<std::string>();
}

inline void applyOption(metkit::mars2grib::Options& opts, std::string_view key, const eckit::Value& value) {
    if (key == "applyChecks") {
        opts.applyChecks = readBool(key, value);
        return;
    }

    if (key == "enableOverride") {
        opts.enableOverride = readBool(key, value);
        return;
    }

    if (key == "enableBitsPerValueCompression") {
        opts.enableBitsPerValueCompression = readBool(key, value);
        return;
    }

    if (key == "normalizeMars") {
        opts.normalizeMars = readBool(key, value);
        return;
    }

    if (key == "normalizeMisc") {
        opts.normalizeMisc = readBool(key, value);
        return;
    }

    if (key == "fixMarsGrid") {
        opts.fixMarsGrid = readBool(key, value);
        return;
    }

    if (key == "skipSection3") {
        opts.skipSection3 = readBool(key, value);
        return;
    }

    if (key == "saveErrorStack") {
        opts.saveErrorStack = readBool(key, value);
        return;
    }

    if (key == "errorStackPath") {
        opts.errorStackPath = readString(key, value);
        return;
    }

    if (key == "printErrorStackToStdErr") {
        opts.printErrorStackToStdErr = readBool(key, value);
        return;
    }

    if (key == "allowDefaultTimeIncrement") {
        opts.allowDefaultTimeIncrement = readBool(key, value);
        return;
    }

    if (key == "allowZeroLengthFsWindow") {
        opts.allowZeroLengthFsWindow = readBool(key, value);
        return;
    }

    if (key == "allowExtendedSetOfOperationsForZeroLengthFsWindow") {
        opts.allowExtendedSetOfOperationsForZeroLengthFsWindow = readBool(key, value);
        return;
    }

    if (key == "allowNonEnumeratedPositiveIntegerTimespanHours") {
        opts.allowNonEnumeratedPositiveIntegerTimespanHours = readBool(key, value);
        return;
    }

    if (key == "allowRedundantTimeIncrement") {
        opts.allowRedundantTimeIncrement = readBool(key, value);
        return;
    }

    if (key == "allowMissingTimespanForInstantProduct") {
        opts.allowMissingTimespanForInstantProduct = readBool(key, value);
        return;
    }

    if (key == "allowMissingTimespanForStatisticalProduct") {
        opts.allowMissingTimespanForStatisticalProduct = readBool(key, value);
        return;
    }

    throw exceptions::Mars2GribDictException("Unknown Mars2Grib option `" + std::string(key) + "`", Here());
}

inline metkit::mars2grib::Options readOptions(metkit::mars2grib::Mars2Grib::OptionList entries) {
    metkit::mars2grib::Options opts;
    std::unordered_set<std::string> seen;
    seen.reserve(entries.size());

    for (const auto& [key, value] : entries) {
        if (!seen.insert(key).second) {
            throw exceptions::Mars2GribDictException("Duplicate Mars2Grib option `" + key + "`", Here());
        }

        applyOption(opts, key, value);
    }

    return opts;
}

}  // namespace metkit::mars2grib::detail
