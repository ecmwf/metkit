// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#include <cmath>
#include <cstdlib>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


static const std::vector<double> randomPercentage() {
    std::vector<double> vals(1661440);
    std::mt19937 rng(42);
    std::uniform_real_distribution<double> dist(0.0, 100.0);
    for (auto& v : vals) {
        v = dist(rng);
    }
    return vals;
}

eckit::LocalConfiguration mars(long param) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", "ep");
    mars.set("expver", "0001");
    mars.set("grid", "O640");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'10'28);
    mars.set("time", 12'00);
    mars.set("step", 168);
    return mars;
}

CASE("encoding") {
    struct test_t {
        std::string name;
        long paramId;
    } tests[]{
        {"Probability of a tropical storm", 131089},
        {"Probability of a hurricane", 131090},
        {"Probability of a tropical depression", 131091},
    };

    for (const auto& [name, paramId] : tests) {
        SECTION(name + " (" + std::to_string(paramId) + ")") {
            const auto handle = metkit::mars2grib::Mars2Grib().encode(randomPercentage(), mars(paramId));

            // MARS
            EXPECT(handle->getLong("param") == paramId);

            // Template number
            EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 121);

            // Ensemble
            EXPECT_EQUAL(handle->getLong("typeOfEnsembleForecast"), 6);  // Perturbed forecast
            EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);

            // Probability
            EXPECT_EQUAL(handle->getLong("forecastProbabilityNumber"), 0);           // Default
            EXPECT_EQUAL(handle->getLong("totalNumberOfForecastProbabilities"), 1);  // Default
            EXPECT_EQUAL(handle->getLong("probabilityType"), 9);  // Probability based on counts of categorical boolean
            EXPECT(handle->isMissing("scaleFactorOfLowerLimit"));
            EXPECT(handle->isMissing("scaledValueOfLowerLimit"));
            EXPECT(handle->isMissing("scaleFactorOfUpperLimit"));
            EXPECT(handle->isMissing("scaledValueOfUpperLimit"));

            // Focal Statistics
            EXPECT_EQUAL(handle->getLong("spatialVicinityType"), 0);  // Circle (in meters)
            EXPECT_EQUAL(handle->getLong("numberOfSpatialVicinityValues"), 1);
            EXPECT_EQUAL(handle->getLong("spatialVicinityValue"), 300'000);   // 300000 M aka 300 KM
            EXPECT_EQUAL(handle->getLong("spatialVicinityProcessing"), 191);  // Categorical (boolean)
            EXPECT(handle->isMissing("spatialVicinityProcessingArgument1"));
            EXPECT(handle->isMissing("spatialVicinityProcessingArgument2"));
            EXPECT_EQUAL(handle->getLong("spatialVicinityMissingData"), 0);     // Ignore missing data
            EXPECT_EQUAL(handle->getLong("temporalVicinityProcessing"), 191);   // Categorical (boolean)
            EXPECT_EQUAL(handle->getLong("temporalVicinityUnit"), 1);           // Hour
            EXPECT_EQUAL(handle->getLong("temporalVicinityTowardsPast"), 48);   // 48 Hours
            EXPECT_EQUAL(handle->getLong("temporalVicinityTowardsFuture"), 0);  // 0 Hours
        }
    }
}

CASE("encoding with misc overrides") {
    eckit::LocalConfiguration misc;
    misc.set("forecastProbabilityNumber", 2);
    misc.set("totalNumberOfForecastProbabilities", 3);

    const auto handle = metkit::mars2grib::Mars2Grib().encode(randomPercentage(), mars(131091), misc);

    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 121);
    EXPECT_EQUAL(handle->getLong("forecastProbabilityNumber"), 2);
    EXPECT_EQUAL(handle->getLong("totalNumberOfForecastProbabilities"), 3);
}


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
