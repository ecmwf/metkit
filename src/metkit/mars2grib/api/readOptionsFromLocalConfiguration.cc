/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

///
/// @file readOptionsFromLocalConfiguration.cc
/// @brief Parse Mars2Grib Options from eckit::LocalConfiguration.
///
/// This file is intended to be merged into the Mars2Grib API implementation.
/// Only explicitly present configuration keys overwrite the defaults declared
/// by `Options`.
///
#include <string>
#include <string_view>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/mars2grib/api/Options.h"
#include "metkit/mars2grib/backend/tables/typeOfTimeIntervals.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib {

namespace {

namespace tables     = metkit::mars2grib::backend::tables;
namespace dict       = metkit::mars2grib::utils::dict_traits;
namespace exceptions = metkit::mars2grib::utils::exceptions;


///
/// @brief Read Mars2Grib options from an eckit configuration.
///
/// A default-constructed `Options` object is created first. Every explicitly
/// present key is then read with its required type and applied to that object.
///
/// The untyped `has(conf, key)` check is intentional. It distinguishes an absent
/// key from a present key with the wrong type. A present key with the wrong type
/// reaches `get_or_throw<T>()` and produces a hard configuration error instead
/// of silently preserving the default.
///
/// @param[in] conf Configuration containing zero or more Mars2Grib options.
///
/// @return A complete strongly typed Options object.
///
Options readOptions(const eckit::LocalConfiguration& conf) {
    Options opts;

    if (dict::has(conf, "applyChecks")) {
        opts.applyChecks = dict::get_or_throw<bool>(conf, "applyChecks");
    }

    if (dict::has(conf, "enableOverride")) {
        opts.enableOverride = dict::get_or_throw<bool>(conf, "enableOverride");
    }

    if (dict::has(conf, "enableBitsPerValueCompression")) {
        opts.enableBitsPerValueCompression = dict::get_or_throw<bool>(conf, "enableBitsPerValueCompression");
    }

    if (dict::has(conf, "normalizeMars")) {
        opts.normalizeMars = dict::get_or_throw<bool>(conf, "normalizeMars");
    }

    if (dict::has(conf, "normalizeMisc")) {
        opts.normalizeMisc = dict::get_or_throw<bool>(conf, "normalizeMisc");
    }

    if (dict::has(conf, "fixMarsGrid")) {
        opts.fixMarsGrid = dict::get_or_throw<bool>(conf, "fixMarsGrid");
    }

    if (dict::has(conf, "skipSection3")) {
        opts.skipSection3 = dict::get_or_throw<bool>(conf, "skipSection3");
    }

    if (dict::has(conf, "allowDefaultTimeIncrementInSeconds")) {
        opts.allowDefaultTimeIncrementInSeconds = dict::get_or_throw<bool>(conf, "allowDefaultTimeIncrementInSeconds");
    }

    if (dict::has(conf, "allowZeroLengthFsWindow")) {
        opts.allowZeroLengthFsWindow = dict::get_or_throw<bool>(conf, "allowZeroLengthFsWindow");
    }

    if (dict::has(conf, "allowExtendedSetOfOperationsForZeroLengthFsWindow")) {
        opts.allowExtendedSetOfOperationsForZeroLengthFsWindow =
            dict::get_or_throw<bool>(conf, "allowExtendedSetOfOperationsForZeroLengthFsWindow");
    }

    if (dict::has(conf, "allowNonEnumeratedPositiveIntegerTimespanHours")) {
        opts.allowNonEnumeratedPositiveIntegerTimespanHours =
            dict::get_or_throw<bool>(conf, "allowNonEnumeratedPositiveIntegerTimespanHours");
    }

    if (dict::has(conf, "allowRedundantTimeIncrement")) {
        opts.allowRedundantTimeIncrement = dict::get_or_throw<bool>(conf, "allowRedundantTimeIncrement");
    }

    if (dict::has(conf, "allowMissingTimespanForInstantProduct")) {
        opts.allowMissingTimespanForInstantProduct =
            dict::get_or_throw<bool>(conf, "allowMissingTimespanForInstantProduct");
    }

    return opts;
}

}  // namespace

}  // namespace metkit::mars2grib