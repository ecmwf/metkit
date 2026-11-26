// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <cmath>
#include <cstdlib>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


static const std::vector<double> vals() {
    std::vector<double> vals(1661440);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    for (auto& v : vals) {
        v = dist(rng);
    }
    return vals;
}

eckit::LocalConfiguration mars(const long param, const std::string& levtype, const std::string& timespan) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", "ep");
    mars.set("expver", "0001");
    mars.set("grid", "O640");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", levtype);
    if (levtype == "pl") {
        mars.set("levelist", 850);
    }
    mars.set("date", 2026'11'26);
    mars.set("time", 00'00);
    mars.set("step", 360);
    mars.set("timespan", timespan);
    return mars;
}

// Note: Ranges 131089-131091 and 131020-131025 are already tested in 015 and 016
// Note: 131073 can be point-in-time, or time-range

CASE("Event Probability Products") {
    const struct test_t {
        long param;
        std::string levtype;
        std::string timespan;
        std::optional<long> typeOfStatisticalProcessing = std::nullopt;
    } tests[]{
        {131060, "sfc", "24h", 1},  {131061, "sfc", "24h", 1}, {131062, "sfc", "24h", 1},  {131063, "sfc", "24h", 1},
        {131064, "sfc", "24h", 1},  {131065, "sfc", "24h", 0}, {131066, "sfc", "24h", 0},  {131067, "sfc", "24h", 0},
        {131068, "sfc", "none"},    {131069, "sfc", "none"},   {131070, "sfc", "120h", 2}, {131071, "sfc", "120h", 2},
        {131072, "sfc", "120h", 2}, {131073, "sfc", "none"},   {131073, "sfc", "120h", 3}, {131085, "sfc", "24h", 1},
        {131098, "sfc", "24h", 1},  {131099, "sfc", "24h", 1}, {131100, "sfc", "24h", 2},  {133093, "pl", "none"},
        {133094, "pl", "none"},     {133095, "pl", "none"},    {133096, "pl", "none"},     {133097, "pl", "none"},
        {133098, "pl", "none"},
    };

    for (const auto& test : tests) {
        SECTION("param=" + std::to_string(test.param) + ", timespan=" + test.timespan +
                ", typeOfStatisticalProcessing=" +
                (test.typeOfStatisticalProcessing ? std::to_string(*test.typeOfStatisticalProcessing) : "MISSING")) {

            const auto handle =
                metkit::mars2grib::Mars2Grib().encode(vals(), mars(test.param, test.levtype, test.timespan));

            // MARS
            EXPECT_EQUAL(handle->getLong("param"), test.param);
            EXPECT_EQUAL(handle->getString("levtype"), test.levtype);
            EXPECT_EQUAL(handle->getLong("step"), 360);
            EXPECT_EQUAL(handle->getString("timespan"), test.timespan);

            // GRIB
            if (test.typeOfStatisticalProcessing) {
                EXPECT_EQUAL(handle->getLong("typeOfStatisticalProcessing"), *test.typeOfStatisticalProcessing);
            }
        }
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
