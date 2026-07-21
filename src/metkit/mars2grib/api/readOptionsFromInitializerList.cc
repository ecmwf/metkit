/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file readOptionsFromInitializerList.cc
/// @brief Parse Mars2Grib Options from a compact initializer list.
///
/// The supported public representation is:
///
/// @code
/// using OptionEntry = std::pair<std::string, eckit::Value>;
/// using OptionList  = std::initializer_list<OptionEntry>;
/// @endcode
///
/// This permits calls such as:
///
/// @code
/// Mars2Grib{{
///     {"skipSection3", true},
///     {"allowDefaultTimeIncrementInSeconds", true},
///     {"defaultTimeIncrementInSeconds", 3600L},
///     {"defaultTypeOfTimeIncrement",
///      "same-start-time-forecast-incremented"}
/// }};
/// @endcode
///
/// `eckit::Value` is used deliberately:
///
/// - boolean values remain distinguishable from integers;
/// - integral values can be validated before conversion to `long`;
/// - string literals remain strings rather than accidentally selecting a
///   boolean alternative in a `std::variant`;
/// - the public initializer syntax stays compact.
///
#include <initializer_list>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>

#include "eckit/value/Value.h"

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib {

namespace {

namespace tables = metkit::mars2grib::backend::tables;
namespace exceptions = metkit::mars2grib::utils::exceptions;

using OptionEntry = std::pair<std::string, eckit::Value>;
using OptionList  = std::initializer_list<OptionEntry>;

[[noreturn]] void throwInvalidOptionType(
    std::string_view key,
    const eckit::Value& value,
    std::string_view expected) {

    throw exceptions::Mars2GribDictException(
        "Option `" + std::string(key) + "` has value type `" +
            value.typeName() + "`; expected " + std::string(expected),
        Here());
}

bool readBool(
    std::string_view key,
    const eckit::Value& value) {

    if (!value.isBool()) {
        throwInvalidOptionType(key, value, "bool");
    }

    return value.as<bool>();
}

long readLong(
    std::string_view key,
    const eckit::Value& value) {

    if (!value.isNumber()) {
        throwInvalidOptionType(key, value, "integral value");
    }

    return value.as<long>();
}

///
/// @brief Apply one initializer-list entry to an Options object.
///
/// Unknown keys are rejected. This avoids silently accepting misspelled option
/// names.
///
/// The function performs representation-level type checks only. Semantic
/// relationships between options, such as requiring a positive
/// `defaultTimeIncrementInSeconds` when defaulting is enabled, remain the
/// responsibility of the ProductTimeSpec extraction/validation path.
///
void applyOption(
    Options& opts,
    std::string_view key,
    const eckit::Value& value) {

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

    if (key == "allowDefaultTimeIncrementInSeconds") {
        opts.allowDefaultTimeIncrementInSeconds = readBool(key, value);
        return;
    }

    if (key == "defaultTimeIncrementInSeconds") {
        opts.defaultTimeIncrementInSeconds = readLong(key, value);
        return;
    }

    if (key == "allowZeroLengthFsWindow") {
        opts.allowZeroLengthFsWindow = readBool(key, value);
        return;
    }

    if (key == "allowNonEnumeratedPositiveIntegerTimespanHours") {
        opts.allowNonEnumeratedPositiveIntegerTimespanHours =
            readBool(key, value);
        return;
    }

    if (key == "allowRedundantTimeIncrement") {
        opts.allowRedundantTimeIncrement = readBool(key, value);
        return;
    }

    if (key == "allowMissingTimespanForInstantProduct") {
        opts.allowMissingTimespanForInstantProduct =
            readBool(key, value);
        return;
    }

    throw exceptions::Mars2GribDictException(
        "Unknown Mars2Grib option `" + std::string(key) + "`",
        Here());
}

///
/// @brief Read Mars2Grib options from an initializer list.
///
/// A default-constructed Options object is created first. Each entry then
/// overwrites the corresponding member.
///
/// Duplicate keys are rejected rather than applying last-value-wins semantics.
/// Duplicate rejection makes accidental contradictory configuration visible at
/// the API boundary.
///
/// @param[in] entries Initializer-list option entries.
///
/// @return A complete strongly typed Options object.
///
Options readOptions(OptionList entries) {
    Options opts;
    std::unordered_set<std::string> seen;
    seen.reserve(entries.size());

    for (const auto& [key, value] : entries) {
        if (!seen.insert(key).second) {
            throw exceptions::Mars2GribDictException(
                "Duplicate Mars2Grib option `" + key + "`",
                Here());
        }

        applyOption(opts, key, value);
    }

    return opts;
}

}  // namespace

}  // namespace metkit::mars2grib

/*
 * Public API integration:
 *
 * In Mars2Grib.h:
 *
 *   #include <initializer_list>
 *   #include <string>
 *   #include <utility>
 *   #include "eckit/value/Value.h"
 *
 *   using OptionEntry = std::pair<std::string, eckit::Value>;
 *   using OptionList  = std::initializer_list<OptionEntry>;
 *
 *   explicit Mars2Grib(OptionList opts);
 *
 * In Mars2Grib.cc, after the anonymous-namespace readOptions overload:
 *
 *   Mars2Grib::Mars2Grib(OptionList opts) :
 *       opts_{readOptions(opts)} {}
 */