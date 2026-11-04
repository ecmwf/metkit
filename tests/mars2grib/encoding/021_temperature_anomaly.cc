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

eckit::LocalConfiguration mars(long param, const std::string& timespan = "none") {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", "ep");
    mars.set("expver", "0001");
    mars.set("grid", "O640");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", "pl");
    mars.set("levelist", 850);
    mars.set("date", 2026'11'04);
    mars.set("time", 00'00);
    mars.set("step", 360);
    mars.set("timespan", timespan);
    return mars;
}

eckit::LocalConfiguration misc(long sampleSizeOfModelClimate, long numberOfDaysInClimateSamplingWindow) {
    eckit::LocalConfiguration misc;
    misc.set("forecastProbabilityNumber", 0);           // Default
    misc.set("totalNumberOfForecastProbabilities", 1);  // Default
    misc.set("yearOfStartOfReferencePeriod", 2006);
    misc.set("monthOfStartOfReferencePeriod", 11);
    misc.set("dayOfStartOfReferencePeriod", 1);
    misc.set("hourOfStartOfReferencePeriod", 0);    // Default
    misc.set("minuteOfStartOfReferencePeriod", 0);  // Default
    misc.set("secondOfStartOfReferencePeriod", 0);  // Default
    misc.set("sampleSizeOfModelClimate", sampleSizeOfModelClimate);
    misc.set("numberOfReforecastYearsInModelClimate", 20);
    misc.set("numberOfDaysInClimateSamplingWindow", numberOfDaysInClimateSamplingWindow);
    return misc;
}

CASE("time-range temperature anomaly") {
    const struct test_t {
        std::string name;
        long paramId;
        long probabilityType;
        long lowerLimit;
    } tests[]{
        {"Temperature anomaly less than -2 K", 131020, 0, 2},
        {"Temperature anomaly of at least +2 K", 131021, 3, 2},
    };

    for (const auto& test : tests) {
        const auto& name           = test.name;
        const auto paramId         = test.paramId;
        const auto probabilityType = test.probabilityType;
        const auto lowerLimit      = test.lowerLimit;

        SECTION(name + " (" + std::to_string(paramId) + ")") {
            const auto handle =
                metkit::mars2grib::Mars2Grib().encode(randomPercentage(), mars(paramId, "120h"), misc(220, 5));

            // MARS
            EXPECT(handle->getLong("param") == paramId);
            EXPECT(handle->getLong("timespan") == 120);

            // Template number
            EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 112);

            // Statistical
            EXPECT_EQUAL(handle->getLong("typeOfStatisticalProcessing"), 0);  // Average

            // Probability
            EXPECT_EQUAL(handle->getLong("forecastProbabilityNumber"), 0);           // Default
            EXPECT_EQUAL(handle->getLong("totalNumberOfForecastProbabilities"), 1);  // Default
            EXPECT_EQUAL(handle->getLong("probabilityType"), probabilityType);
            EXPECT_EQUAL(handle->getLong("scaleFactorOfLowerLimit"), 0);
            EXPECT_EQUAL(handle->getLong("scaledValueOfLowerLimit"), lowerLimit);
            EXPECT(handle->isMissing("scaleFactorOfUpperLimit"));
            EXPECT(handle->isMissing("scaledValueOfUpperLimit"));

            // Reference Period
            EXPECT_EQUAL(handle->getLong("typeOfReferenceDataset"), 2);            // Reforecast (Hindcast)
            EXPECT_EQUAL(handle->getLong("typeOfRelationToReferenceDataset"), 0);  // Anomaly
            EXPECT_EQUAL(handle->getLong("numberOfAdditionalParametersForReferencePeriod"), 0);  // None
            // No additional parameters to check...
            EXPECT_EQUAL(handle->getLong("yearOfStartOfReferencePeriod"), 2006);
            EXPECT_EQUAL(handle->getLong("monthOfStartOfReferencePeriod"), 11);
            EXPECT_EQUAL(handle->getLong("dayOfStartOfReferencePeriod"), 1);
            EXPECT_EQUAL(handle->getLong("hourOfStartOfReferencePeriod"), 0);
            EXPECT_EQUAL(handle->getLong("minuteOfStartOfReferencePeriod"), 0);
            EXPECT_EQUAL(handle->getLong("secondOfStartOfReferencePeriod"), 0);
            EXPECT_EQUAL(handle->getLong("sampleSizeOfReferencePeriod"), 220);

            // Reference Period Time Ranges
            EXPECT_EQUAL(handle->getLong("numberOfReferencePeriodTimeRanges"), 2);
            const auto& typeOfStatisticalProcessingForTimeRange =
                handle->getLongArray("typeOfStatisticalProcessingForTimeRangeForReferencePeriod");
            const auto& indicatorofUnitForTimeRange =
                handle->getLongArray("indicatorOfUnitForTimeRangeForReferencePeriod");
            const auto& lengthOfTimeRange = handle->getLongArray("lengthOfTimeRangeForReferencePeriod");
            EXPECT_EQUAL(typeOfStatisticalProcessingForTimeRange[0], 20);  // Model climate
            EXPECT_EQUAL(indicatorofUnitForTimeRange[0], 4);               // Year
            EXPECT_EQUAL(lengthOfTimeRange[0], 20);                        // 20 Years
            EXPECT_EQUAL(typeOfStatisticalProcessingForTimeRange[1], 20);  // Model climate
            EXPECT_EQUAL(indicatorofUnitForTimeRange[1], 2);               // Days
            EXPECT_EQUAL(lengthOfTimeRange[1], 5);                         // 5 Days
        }
    }
}

CASE("point-in-time temperature anomaly") {
    const struct test_t {
        std::string name;
        long paramId;
        long probabilityType;
        long lowerLimit;
    } tests[]{
        {"Temperature anomaly less than -8 K", 131022, 0, -8},
        {"Temperature anomaly less than -4 K", 131023, 0, -4},
        {"Temperature anomaly greater than +4 K", 131024, 3, 4},
        {"Temperature anomaly greater than +8 K", 131025, 3, 8},
    };

    for (const auto& test : tests) {
        const auto& name           = test.name;
        const auto paramId         = test.paramId;
        const auto probabilityType = test.probabilityType;
        const auto lowerLimit      = test.lowerLimit;

        SECTION(name + " (" + std::to_string(paramId) + ")") {
            const auto handle =
                metkit::mars2grib::Mars2Grib().encode(randomPercentage(), mars(paramId, "120h"), misc(20, 1));

            // MARS
            EXPECT(handle->getLong("param") == paramId);

            // Template number
            EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 131);

            // Probability
            EXPECT_EQUAL(handle->getLong("forecastProbabilityNumber"), 0);           // Default
            EXPECT_EQUAL(handle->getLong("totalNumberOfForecastProbabilities"), 1);  // Default
            EXPECT_EQUAL(handle->getLong("probabilityType"), probabilityType);
            EXPECT_EQUAL(handle->getLong("scaleFactorOfLowerLimit"), 0);
            EXPECT_EQUAL(handle->getLong("scaledValueOfLowerLimit"), lowerLimit);
            EXPECT(handle->isMissing("scaleFactorOfUpperLimit"));
            EXPECT(handle->isMissing("scaledValueOfUpperLimit"));

            // Reference Period
            EXPECT_EQUAL(handle->getLong("typeOfReferenceDataset"), 2);            // Reforecast (Hindcast)
            EXPECT_EQUAL(handle->getLong("typeOfRelationToReferenceDataset"), 0);  // Anomaly
            EXPECT_EQUAL(handle->getLong("numberOfAdditionalParametersForReferencePeriod"), 0);  // None
            // No additional parameters to check...
            EXPECT_EQUAL(handle->getLong("yearOfStartOfReferencePeriod"), 2006);
            EXPECT_EQUAL(handle->getLong("monthOfStartOfReferencePeriod"), 11);
            EXPECT_EQUAL(handle->getLong("dayOfStartOfReferencePeriod"), 1);
            EXPECT_EQUAL(handle->getLong("hourOfStartOfReferencePeriod"), 0);
            EXPECT_EQUAL(handle->getLong("minuteOfStartOfReferencePeriod"), 0);
            EXPECT_EQUAL(handle->getLong("secondOfStartOfReferencePeriod"), 0);
            EXPECT_EQUAL(handle->getLong("sampleSizeOfReferencePeriod"), 20);

            // Reference Period Time Ranges
            EXPECT_EQUAL(handle->getLong("numberOfReferencePeriodTimeRanges"), 2);
            const auto& typeOfStatisticalProcessingForTimeRange =
                handle->getLongArray("typeOfStatisticalProcessingForTimeRangeForReferencePeriod");
            const auto& indicatorofUnitForTimeRange =
                handle->getLongArray("indicatorOfUnitForTimeRangeForReferencePeriod");
            const auto& lengthOfTimeRange = handle->getLongArray("lengthOfTimeRangeForReferencePeriod");
            EXPECT_EQUAL(typeOfStatisticalProcessingForTimeRange[0], 20);  // Model climate
            EXPECT_EQUAL(indicatorofUnitForTimeRange[0], 4);               // Year
            EXPECT_EQUAL(lengthOfTimeRange[0], 20);                        // 20 Years
            EXPECT_EQUAL(typeOfStatisticalProcessingForTimeRange[1], 20);  // Model climate
            EXPECT_EQUAL(indicatorofUnitForTimeRange[1], 2);               // Days
            EXPECT_EQUAL(lengthOfTimeRange[1], 1);                         // 1 Day
        }
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
