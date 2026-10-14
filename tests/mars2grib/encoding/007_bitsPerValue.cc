// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <cstdlib>
#include <random>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


static const eckit::LocalConfiguration mars = []() {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'18);
    mars.set("time", 00'00);
    mars.set("step", 42);
    return mars;
}();

static const std::vector<double> randomTemps = []() {
    std::vector<double> vals(200);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(237.15 - 40, 237.15 + 40);
    for (auto& v : vals) {
        v = dist(rng);
    }
    return vals;
}();

static const std::vector<double> randomZeroToOne = []() {
    std::vector<double> vals(200);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    for (auto& v : vals) {
        v = dist(rng);
    }
    return vals;
}();


CASE("bitsPerValue=0 (constant)") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    std::vector<double> vals(200, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("bitsPerValue"), 0);
}

CASE("bitsPerValue=16 (default)") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto handle = encoder.encode(randomTemps, mars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("bitsPerValue"), 16);
}

CASE("bitsPerValue=8") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration misc;
    misc.set("bitsPerValue", 8);

    const auto handle = encoder.encode(randomTemps, mars, misc);

    // GRIB
    EXPECT_EQUAL(handle->getLong("bitsPerValue"), 8);
}

CASE("bitsPerValue=24") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration misc;
    misc.set("bitsPerValue", 24);

    const auto handle = encoder.encode(randomTemps, mars, misc);

    // GRIB
    EXPECT_EQUAL(handle->getLong("bitsPerValue"), 24);
}

CASE("bitsPerValue=8 (default for cloud cover)") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    auto customMars = mars;
    customMars.set("param", 248);  // Cloud Cover

    const auto handle = encoder.encode(randomTemps, customMars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("bitsPerValue"), 8);
}

CASE("bitsPerValue=24 (default for 210001 to 227999 range)") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    auto customMars = mars;
    customMars.set("param", 210186);  // UV visible albedo for direct radiation
    customMars.set("levtype", "sfc");
    customMars.remove("levelist");

    const auto handle = encoder.encode(randomZeroToOne, customMars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("bitsPerValue"), 24);
}


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
