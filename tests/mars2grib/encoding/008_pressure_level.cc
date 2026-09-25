/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <cstdlib>
#include <random>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/codes/api/CodesAPI.h"
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

CASE("1000 hPa") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    auto customMars = mars;
    customMars.set("levelist", 1000);  // Unit: hPa

    const auto handle = encoder.encode(randomTemps, customMars);

    // MARS
    EXPECT_EQUAL(handle->getString("levtype"), "pl");
    EXPECT_EQUAL(handle->getDouble("levelist"), 1000);  // Unit: hPa

    // GRIB
    EXPECT_EQUAL(handle->getLong("typeOfFirstFixedSurface"), 100);            // Isobaric surface in Pa
    EXPECT_EQUAL(handle->getLong("scaleFactorOfFirstFixedSurface"), 0);       // Multiply scaledValue by 10^(-0), aka 1
    EXPECT_EQUAL(handle->getLong("scaledValueOfFirstFixedSurface"), 100000);  // Unit: Pa
    EXPECT_EQUAL(handle->getLong("typeOfSecondFixedSurface"), 255);           // Missing
}

CASE("100 hPa") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    auto customMars = mars;
    customMars.set("levelist", 100);  // Unit: hPa

    const auto handle = encoder.encode(randomTemps, customMars);

    // MARS
    EXPECT_EQUAL(handle->getString("levtype"), "pl");
    EXPECT_EQUAL(handle->getDouble("levelist"), 100);  // Unit: hPa

    // GRIB
    EXPECT_EQUAL(handle->getLong("typeOfFirstFixedSurface"), 100);           // Isobaric surface in Pa
    EXPECT_EQUAL(handle->getLong("scaleFactorOfFirstFixedSurface"), 0);      // Multiply scaledValue by 10^(-0), aka 1
    EXPECT_EQUAL(handle->getLong("scaledValueOfFirstFixedSurface"), 10000);  // Unit: Pa
    EXPECT_EQUAL(handle->getLong("typeOfSecondFixedSurface"), 255);          // Missing
}

CASE("10 hPa") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    auto customMars = mars;
    customMars.set("levelist", 10);  // Unit: hPa

    const auto handle = encoder.encode(randomTemps, customMars);

    // MARS
    EXPECT_EQUAL(handle->getString("levtype"), "pl");
    EXPECT_EQUAL(handle->getDouble("levelist"), 10);  // Unit: hPa

    // GRIB
    EXPECT_EQUAL(handle->getLong("typeOfFirstFixedSurface"), 100);          // Isobaric surface in Pa
    EXPECT_EQUAL(handle->getLong("scaleFactorOfFirstFixedSurface"), 0);     // Multiply scaledValue by 10^(-0), aka 1
    EXPECT_EQUAL(handle->getLong("scaledValueOfFirstFixedSurface"), 1000);  // Pa
    EXPECT_EQUAL(handle->getLong("typeOfSecondFixedSurface"), 255);         // Missing
}

CASE("50 Pa (0.5 hPa)") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    auto customMars = mars;
    customMars.set("levelist", 0.5);  // Unit: hPa

    const auto handle = encoder.encode(randomTemps, customMars);

    // MARS
    EXPECT_EQUAL(handle->getString("levtype"), "pl");
    EXPECT_EQUAL(handle->getDouble("levelist"), 0.5);  // Unit: hPa

    // GRIB
    EXPECT_EQUAL(handle->getLong("typeOfFirstFixedSurface"), 100);        // Isobaric surface in Pa
    EXPECT_EQUAL(handle->getLong("scaleFactorOfFirstFixedSurface"), 0);   // Multiply scaledValue by 10^(-0), aka 1
    EXPECT_EQUAL(handle->getLong("scaledValueOfFirstFixedSurface"), 50);  // Unit: Pa
    EXPECT_EQUAL(handle->getLong("typeOfSecondFixedSurface"), 255);       // Missing
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
