/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file readOptionsFromLocalConfiguration.h
/// @brief Parse Mars2Mars Options from eckit::LocalConfiguration.
///
/// This file is intended to be included into `Mars2Mars.cc` so the parser can
/// remain implementation-only while still being the single source of truth.
///

#include <string>
#include <string_view>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2mars/api/Options.h"
#include "metkit/mars2mars/utils/mars2marsExceptions.h"

namespace metkit::mars2mars::detail {

inline bool readBoolOption(const eckit::LocalConfiguration& conf, std::string_view key) {
    if (!conf.isBoolean(std::string{key})) {
        throw utils::exceptions::Mars2marsDictException(
            "Option `" + std::string(key) + "` must be a bool in eckit::LocalConfiguration", Here());
    }

    return conf.getBool(std::string{key});
}

inline std::string readStringOption(const eckit::LocalConfiguration& conf, std::string_view key) {
    if (!conf.isString(std::string{key})) {
        throw utils::exceptions::Mars2marsDictException(
            "Option `" + std::string(key) + "` must be a string in eckit::LocalConfiguration", Here());
    }

    return conf.getString(std::string{key});
}

inline Options readOptions(const eckit::LocalConfiguration& conf) {
    Options opts;

    if (conf.has("saveErrorStack")) {
        opts.saveErrorStack = readBoolOption(conf, "saveErrorStack");
    }

    if (conf.has("skipSection3")) {
        opts.skipSection3 = readBoolOption(conf, "skipSection3");
    }

    if (conf.has("errorStackPath")) {
        opts.errorStackPath = readStringOption(conf, "errorStackPath");
    }

    if (conf.has("printErrorStackToStdErr")) {
        opts.printErrorStackToStdErr = readBoolOption(conf, "printErrorStackToStdErr");
    }

    if (conf.has("tryFixBadInput_ZeroAccumulation")) {
        opts.tryFixBadInput_ZeroAccumulation = readBoolOption(conf, "tryFixBadInput_ZeroAccumulation");
    }

    return opts;
}

}  // namespace metkit::mars2mars::detail
