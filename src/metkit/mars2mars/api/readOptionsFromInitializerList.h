/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file readOptionsFromInitializerList.h
/// @brief Parse Mars2Mars Options from a compact initializer list.
///
/// This file is intended to be included into `Mars2Mars.cc` so the parser can
/// remain implementation-only while still being the single source of truth.
///

#include <string_view>
#include <unordered_set>

#include "eckit/value/Value.h"

#include "metkit/mars2mars/api/Mars2Mars.h"
#include "metkit/mars2mars/api/Options.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::detail {

namespace exceptions = metkit::mars2mars::utils::exceptions;

[[noreturn]] inline void throwInvalidOptionType(std::string_view key, const eckit::Value& value,
                                                std::string_view expected) {
    throw exceptions::Mars2marsDictException("Option `" + std::string(key) + "` has value type `" + value.typeName() +
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

inline void applyOption(metkit::mars2mars::Options& opts, std::string_view key, const eckit::Value& value) {
    if (key == "saveErrorStack") {
        opts.saveErrorStack = readBool(key, value);
        return;
    }

    if (key == "skipSection3") {
        opts.skipSection3 = readBool(key, value);
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

    if (key == "tryFixBadInput_ZeroAccumulation") {
        opts.tryFixBadInput_ZeroAccumulation = readBool(key, value);
        return;
    }

    throw exceptions::Mars2marsDictException("Unknown Mars2Mars option `" + std::string(key) + "`", Here());
}

inline metkit::mars2mars::Options readOptions(metkit::mars2mars::Mars2Mars::OptionList entries) {
    metkit::mars2mars::Options opts;
    std::unordered_set<std::string> seen;
    seen.reserve(entries.size());

    for (const auto& [key, value] : entries) {
        if (!seen.insert(key).second) {
            throw exceptions::Mars2marsDictException("Duplicate Mars2Mars option `" + key + "`", Here());
        }

        applyOption(opts, key, value);
    }

    return opts;
}

}  // namespace metkit::mars2mars::detail
