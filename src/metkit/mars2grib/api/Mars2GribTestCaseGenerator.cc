/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "Mars2GribTestCaseGenerator.h"

#include <string_view>
#include <unordered_set>

#include "metkit/mars2grib/CoreOperations.h"
#include "metkit/mars2grib/api/Mars2GribApiErrorHandling.h"
#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"
#include "metkit/mars2grib/testing-utils/dictionary_traits/dictaccess_recording_dictionary.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_options.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib {

namespace exceptions = metkit::mars2grib::utils::exceptions;

namespace {

[[noreturn]] void throwInvalidOptionType(std::string_view key, const eckit::Value& value, std::string_view expected) {
    throw exceptions::Mars2GribDictException("Option `" + std::string(key) + "` has value type `" + value.typeName() +
                                                 "`; expected " + std::string(expected),
                                             Here());
}

bool readBool(const eckit::Value& value, std::string_view key) {
    if (!value.isBool()) {
        throwInvalidOptionType(key, value, "bool");
    }

    return value.as<bool>();
}

std::string readString(const eckit::Value& value, std::string_view key) {
    if (!value.isString()) {
        throwInvalidOptionType(key, value, "string");
    }

    return value.as<std::string>();
}

void applyOption(Options& opts, std::string_view key, const eckit::Value& value) {
    if (key == "applyChecks") {
        opts.applyChecks = readBool(value, key);
        return;
    }

    if (key == "enableOverride") {
        opts.enableOverride = readBool(value, key);
        return;
    }

    if (key == "enableBitsPerValueCompression") {
        opts.enableBitsPerValueCompression = readBool(value, key);
        return;
    }

    if (key == "normalizeMars") {
        opts.normalizeMars = readBool(value, key);
        return;
    }

    if (key == "normalizeMisc") {
        opts.normalizeMisc = readBool(value, key);
        return;
    }

    if (key == "fixMarsGrid") {
        opts.fixMarsGrid = readBool(value, key);
        return;
    }

    if (key == "skipSection3") {
        opts.skipSection3 = readBool(value, key);
        return;
    }

    if (key == "saveErrorStack") {
        opts.saveErrorStack = readBool(value, key);
        return;
    }

    if (key == "errorStackPath") {
        opts.errorStackPath = readString(value, key);
        return;
    }

    if (key == "printErrorStackToStdErr") {
        opts.printErrorStackToStdErr = readBool(value, key);
        return;
    }

    if (key == "allowDefaultTimeIncrement") {
        opts.allowDefaultTimeIncrement = readBool(value, key);
        return;
    }

    if (key == "allowZeroLengthFsWindow") {
        opts.allowZeroLengthFsWindow = readBool(value, key);
        return;
    }

    if (key == "allowExtendedSetOfOperationsForZeroLengthFsWindow") {
        opts.allowExtendedSetOfOperationsForZeroLengthFsWindow = readBool(value, key);
        return;
    }

    if (key == "allowNonEnumeratedPositiveIntegerTimespanHours") {
        opts.allowNonEnumeratedPositiveIntegerTimespanHours = readBool(value, key);
        return;
    }

    if (key == "allowRedundantTimeIncrement") {
        opts.allowRedundantTimeIncrement = readBool(value, key);
        return;
    }

    if (key == "allowMissingTimespanForInstantProduct") {
        opts.allowMissingTimespanForInstantProduct = readBool(value, key);
        return;
    }

    if (key == "allowMissingTimespanForStatisticalProduct") {
        opts.allowMissingTimespanForStatisticalProduct = readBool(value, key);
        return;
    }

    throw exceptions::Mars2GribDictException("Unknown Mars2Grib option `" + std::string(key) + "`", Here());
}

Options readOptions(const Mars2GribTestCaseGenerator::OptionList entries) {
    Options opts;
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

bool readBoolOption(const eckit::LocalConfiguration& conf, std::string_view key) {
    if (!conf.isBoolean(std::string{key})) {
        throw exceptions::Mars2GribDictException(
            "Option `" + std::string(key) + "` must be a bool in eckit::LocalConfiguration", Here());
    }

    return conf.getBool(std::string{key});
}

std::string readStringOption(const eckit::LocalConfiguration& conf, std::string_view key) {
    if (!conf.isString(std::string{key})) {
        throw exceptions::Mars2GribDictException(
            "Option `" + std::string(key) + "` must be a string in eckit::LocalConfiguration", Here());
    }

    return conf.getString(std::string{key});
}

Options readOptions(const eckit::LocalConfiguration& conf) {
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

std::string makeTestCaseJson(const eckit::LocalConfiguration& mars, const eckit::LocalConfiguration& misc,
                             const Options& opts, const testing_utils::RecordingDictionary& out) {
    using metkit::mars2grib::utils::dict_traits::dict_to_json;

    std::string json;
    json += "{";
    json += "\"mars\":";
    json += dict_to_json(mars);
    json += ",\"misc\":";
    json += dict_to_json(misc);
    json += ",\"opt\":";
    json += dict_to_json(opts);
    json += ",\"out\":";
    json += out.to_json();
    json += "}";
    return json;
}

}  // namespace

Mars2GribTestCaseGenerator::Mars2GribTestCaseGenerator() : opts_{} {}

Mars2GribTestCaseGenerator::Mars2GribTestCaseGenerator(const Options& opts) : opts_{opts} {}

Mars2GribTestCaseGenerator::Mars2GribTestCaseGenerator(const eckit::LocalConfiguration& opts) :
    opts_{readOptions(opts)} {}

Mars2GribTestCaseGenerator::Mars2GribTestCaseGenerator(OptionList opts) : opts_{readOptions(opts)} {}

std::string Mars2GribTestCaseGenerator::generate(const eckit::LocalConfiguration& mars,
                                                 const eckit::LocalConfiguration& misc) {
    return exceptions::withMars2GribApiErrorHandling<std::string>(
        "Mars2GribTestCaseGenerator::generate", opts_,
        [&]() {
            auto out =
                CoreOperations::encodeHeaderWithNormalization<eckit::LocalConfiguration, eckit::LocalConfiguration,
                                                              Options, testing_utils::RecordingDictionary>(
                    mars, misc, opts_, language_);

            return makeTestCaseJson(mars, misc, opts_, *out);
        },
        Here());
}

std::string Mars2GribTestCaseGenerator::generate(const eckit::LocalConfiguration& mars) {
    const eckit::LocalConfiguration misc{};
    return generate(mars, misc);
}

}  // namespace metkit::mars2grib
