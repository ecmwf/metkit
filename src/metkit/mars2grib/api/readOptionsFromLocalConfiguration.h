/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file readOptionsFromLocalConfiguration.h
/// @brief Parse Mars2Grib Options from eckit::LocalConfiguration.
///
/// This file is intended to be included into `Mars2Grib.cc` so the parser can
/// remain implementation-only while still being the single source of truth.
///

#include <string>
#include <string_view>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::detail {

inline bool readBoolOption(const eckit::LocalConfiguration& conf, std::string_view key) {
    if (!conf.isBoolean(std::string{key})) {
        throw utils::exceptions::Mars2GribDictException(
            "Option `" + std::string(key) + "` must be a bool in eckit::LocalConfiguration", Here());
    }

    return conf.getBool(std::string{key});
}

inline std::string readStringOption(const eckit::LocalConfiguration& conf, std::string_view key) {
    if (!conf.isString(std::string{key})) {
        throw utils::exceptions::Mars2GribDictException(
            "Option `" + std::string(key) + "` must be a string in eckit::LocalConfiguration", Here());
    }

    return conf.getString(std::string{key});
}

inline Options readOptions(const eckit::LocalConfiguration& conf) {
    Options opts;

    if (conf.has("applyChecks")) {
        opts.applyChecks = readBoolOption(conf, "applyChecks");
    }

    if (conf.has("enableOverride")) {
        opts.enableOverride = readBoolOption(conf, "enableOverride");
    }

    if (conf.has("enableBitsPerValueCompression")) {
        opts.enableBitsPerValueCompression = readBoolOption(conf, "enableBitsPerValueCompression");
    }

    if (conf.has("normalizeMars")) {
        opts.normalizeMars = readBoolOption(conf, "normalizeMars");
    }

    if (conf.has("normalizeMisc")) {
        opts.normalizeMisc = readBoolOption(conf, "normalizeMisc");
    }

    if (conf.has("fixMarsGrid")) {
        opts.fixMarsGrid = readBoolOption(conf, "fixMarsGrid");
    }

    if (conf.has("skipSection3")) {
        opts.skipSection3 = readBoolOption(conf, "skipSection3");
    }

    if (conf.has("saveErrorStack")) {
        opts.saveErrorStack = readBoolOption(conf, "saveErrorStack");
    }

    if (conf.has("errorStackPath")) {
        opts.errorStackPath = readStringOption(conf, "errorStackPath");
    }

    if (conf.has("printErrorStackToStdErr")) {
        opts.printErrorStackToStdErr = readBoolOption(conf, "printErrorStackToStdErr");
    }

    if (conf.has("allowDefaultTimeIncrement")) {
        opts.allowDefaultTimeIncrement = readBoolOption(conf, "allowDefaultTimeIncrement");
    }

    if (conf.has("allowZeroLengthFsWindow")) {
        opts.allowZeroLengthFsWindow = readBoolOption(conf, "allowZeroLengthFsWindow");
    }

    if (conf.has("allowExtendedSetOfOperationsForZeroLengthFsWindow")) {
        opts.allowExtendedSetOfOperationsForZeroLengthFsWindow =
            readBoolOption(conf, "allowExtendedSetOfOperationsForZeroLengthFsWindow");
    }

    if (conf.has("allowNonEnumeratedPositiveIntegerTimespanHours")) {
        opts.allowNonEnumeratedPositiveIntegerTimespanHours =
            readBoolOption(conf, "allowNonEnumeratedPositiveIntegerTimespanHours");
    }

    if (conf.has("allowRedundantTimeIncrement")) {
        opts.allowRedundantTimeIncrement = readBoolOption(conf, "allowRedundantTimeIncrement");
    }

    if (conf.has("allowMissingTimespanForInstantProduct")) {
        opts.allowMissingTimespanForInstantProduct = readBoolOption(conf, "allowMissingTimespanForInstantProduct");
    }

    if (conf.has("allowMissingTimespanForStatisticalProduct")) {
        opts.allowMissingTimespanForStatisticalProduct =
            readBoolOption(conf, "allowMissingTimespanForStatisticalProduct");
    }

    return opts;
}

}  // namespace metkit::mars2grib::detail
