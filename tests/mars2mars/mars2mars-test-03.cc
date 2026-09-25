/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file mars2mars-test-03.cc
/// @brief The wave streams are converted to their atmospheric counterparts.

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/mars2mars/api/Mars2Mars.h"

namespace {

eckit::LocalConfiguration waveRequest(const std::string& stream, const std::string& type) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", stream);
    mars.set("type", type);
    mars.set("expver", "0001");
    mars.set("date", 20261001L);
    mars.set("time", 0L);
    mars.set("levtype", "sfc");
    mars.set("param", 140229L);
    mars.set("step", 24L);
    if (type == "pf") {
        mars.set("number", 4L);
    }
    if (stream == "enwh") {
        mars.set("hdate", 20201001L);
    }
    return mars;
}

std::string convertedStream(const eckit::LocalConfiguration& in) {
    try {
        return metkit::mars2mars::Mars2Mars{}.convert(in).mars.getString("stream");
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Test failed with exception: ") + e.what());
    }
}

}  // namespace

CASE("wave is converted to oper") {
    EXPECT_EQUAL(convertedStream(waveRequest("wave", "fc")), "oper");
}

CASE("waef is converted to enfo") {
    EXPECT_EQUAL(convertedStream(waveRequest("waef", "pf")), "enfo");
}

CASE("enwh is converted to enfh, keeping hdate and number") {
    const auto in        = waveRequest("enwh", "pf");
    const auto converted = metkit::mars2mars::Mars2Mars{}.convert(in).mars;

    EXPECT_EQUAL(converted.getString("stream"), "enfh");
    EXPECT_EQUAL(converted.getString("type"), "pf");
    EXPECT_EQUAL(converted.getLong("hdate"), 20201001L);
    EXPECT_EQUAL(converted.getLong("number"), 4L);
    EXPECT_EQUAL(converted.getLong("param"), 140229L);
}

CASE("enwh cf is converted to enfh") {
    EXPECT_EQUAL(convertedStream(waveRequest("enwh", "cf")), "enfh");
}

CASE("every wave stream is converted to its atmospheric counterpart") {
    const std::vector<std::pair<std::string, std::string>> streams{
        {"wave", "oper"}, {"scwv", "scda"}, {"dcwv", "dcda"}, {"lwwv", "lwda"}, {"ewda", "enda"}, {"ewla", "elda"},
        {"fsow", "fsob"}, {"waef", "enfo"}, {"enwh", "enfh"}, {"weef", "eefo"}, {"weeh", "eefh"}, {"ewho", "efho"},
        {"weov", "efov"}, {"ewhc", "efhc"}, {"wehs", "efhs"}, {"wees", "eehs"}, {"wamo", "mnth"}, {"wamd", "moda"},
        {"ewmm", "edmm"}, {"ewmo", "edmo"}, {"dacw", "dacl"},
    };

    for (const auto& stream : streams) {
        EXPECT_EQUAL(convertedStream(waveRequest(stream.first, "fc")), stream.second);
    }
}

CASE("legacy seasonal and monthly forecast wave streams, and streams without a counterpart, are unchanged") {
    for (const std::string stream : {"wasf", "swmm", "wams", "mswm", "mmaw", "mmwm", "wamf", "wmfm", "mnfw", "mfhw",
                                     "mfaw", "mfwm", "mhwm", "mawm", "mawv", "wvhc", "wavm"}) {
        EXPECT_EQUAL(convertedStream(waveRequest(stream, "fc")), stream);
    }
}

CASE("atmospheric streams are unchanged") {
    EXPECT_EQUAL(convertedStream(waveRequest("oper", "fc")), "oper");
    EXPECT_EQUAL(convertedStream(waveRequest("enfo", "pf")), "enfo");
    EXPECT_EQUAL(convertedStream(waveRequest("enfh", "pf")), "enfh");
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
