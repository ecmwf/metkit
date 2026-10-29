/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <memory>
#include <string>

#include "eckit/config/LocalConfiguration.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/testing-utils/RecordingDictionary.h"

namespace metkit::mars2grib::testing_utils::run_tests {

/// Replay a recorded set-list on a real GRIB handle: make_from_sample -> codes sample, set ->
/// CodesHandle::set (booleans as 0/1), set_missing -> setMissing, clone -> clone(). A recorded
/// `values` summary is replayed as a constant field of the recorded size and average.
std::unique_ptr<metkit::codes::CodesHandle> replayOnCodesHandle(const RecordingDictionary& recording);

struct DecodedTestCase {
    eckit::LocalConfiguration mars;
    eckit::LocalConfiguration misc;
};

/// Replay `recording` and convert the resulting GRIB message back with grib2mars.
/// `options` are the mars2grib options of the test; `skipSection3` is passed on to grib2mars.
DecodedTestCase decodeRecording(const RecordingDictionary& recording, const eckit::LocalConfiguration& options);

struct DecodeComparison {
    bool equal{true};
    /// One line per mismatching key.
    std::string diff;
};

/// Every key of the expected mars and misc must be present in the decoded ones with an equal
/// value; extra decoded keys are ignored. Strings compare case-insensitively, numbers by value
/// (also when one side is a numeric string, e.g. "130" and 130), lists element by element, and
/// a scalar equals a one-element list. With `compareMisc` false only mars is compared.
DecodeComparison compareDecoded(const eckit::LocalConfiguration& expectedMars,
                                const eckit::LocalConfiguration& expectedMisc, const DecodedTestCase& decoded,
                                bool compareMisc = true);

}  // namespace metkit::mars2grib::testing_utils::run_tests
