/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <array>
#include <exception>
#include <string>
#include <vector>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/api/Mars2GribClassify.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

// EFI / SOT against the model climate, as in od:0001:enfo after mars2mars (step range -> step + timespan,
// SOT `number` -> `quantile`)
eckit::LocalConfiguration indexRequest(const std::string& type, long param, long step, long timespan) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", type);
    mars.set("expver", "0001");
    mars.set("domain", "g");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'09'30);
    mars.set("time", 00'00);
    mars.set("step", step);
    mars.set("timespan", timespan);
    return mars;
}

// Model climate of GRIB1 local definition 19, forwarded by grib2mars
eckit::LocalConfiguration climateMisc(long numberOfForecastsInEnsemble) {
    eckit::LocalConfiguration misc;
    misc.set("numberOfForecastsInEnsemble", numberOfForecastsInEnsemble);
    misc.set("numberOfReforecastYearsInModelClimate", 20L);
    misc.set("numberOfDaysInClimateSamplingWindow", 31L);
    misc.set("sampleSizeOfModelClimate", 1980L);
    return misc;
}

template <typename Handle>
void expectModelClimate(const Handle& handle, long typeOfRelationToReferenceDataset) {
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 107);
    EXPECT_EQUAL(handle->getLong("typeOfGeneratingProcess"), 4);
    EXPECT_EQUAL(handle->getLong("derivedForecast"), 5);
    EXPECT_EQUAL(handle->getLong("typeOfReferenceDataset"), 2);
    EXPECT_EQUAL(handle->getLong("typeOfRelationToReferenceDataset"), typeOfRelationToReferenceDataset);
    EXPECT_EQUAL(handle->getLong("sampleSizeOfReferencePeriod"), 1980);
    EXPECT_EQUAL(handle->getLong("numberOfReferencePeriodTimeRanges"), 2);
    EXPECT(handle->getLongArray("indicatorOfUnitForTimeRangeForReferencePeriod") == (std::vector<long>{4, 2}));
    EXPECT(handle->getLongArray("lengthOfTimeRangeForReferencePeriod") == (std::vector<long>{20, 31}));
    // Code table 4.102: 20 = Model climate
    EXPECT(handle->getLongArray("typeOfStatisticalProcessingForTimeRangeForReferencePeriod") ==
           (std::vector<long>{20, 20}));
    // The start of the model climate has no meaningful date
    EXPECT(handle->isMissing("yearOfStartOfReferencePeriod"));
}

template <typename Handle>
void expectIndexTimeRanges(const Handle& handle, long inner, long startStep, long length) {
    EXPECT_EQUAL(handle->getLong("numberOfTimeRanges"), 2);
    EXPECT(handle->getLongArray("typeOfStatisticalProcessing") == (std::vector<long>{102, inner}));
    EXPECT(handle->getLongArray("lengthOfTimeRange") == (std::vector<long>{length, length}));
    EXPECT_EQUAL(handle->getLong("forecastTime"), startStep);
}

template <typename F>
void run(F&& f) {
    try {
        f();
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

}  // namespace

CASE("enfo efi: two time ranges [102, average] and EFI model climate") {
    run([] {
        auto encoder = metkit::mars2grib::Mars2Grib();
        const auto handle =
            encoder.encode(std::vector<double>(200, 0.5), indexRequest("efi", 132045, 360, 360), climateMisc(51));

        expectModelClimate(handle, 20);
        expectIndexTimeRanges(handle, 0, 0, 360);
        EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);
        EXPECT_EQUAL(handle->getLong("numberOfAdditionalParametersForReferencePeriod"), 0);
        EXPECT_EQUAL(handle->getString("mars.type"), "efi");
        EXPECT_EQUAL(handle->getLong("mars.step"), 360);
        EXPECT_EQUAL(handle->getLong("typeOfProcessedData"), 5);  // control and perturbed
    });
}

CASE("enfo efic: control EFI has one forecast in the ensemble") {
    run([] {
        auto encoder = metkit::mars2grib::Mars2Grib();
        const auto handle =
            encoder.encode(std::vector<double>(200, 0.5), indexRequest("efic", 132167, 120, 24), climateMisc(1));

        expectModelClimate(handle, 20);
        expectIndexTimeRanges(handle, 0, 96, 24);
        EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 1);
        EXPECT_EQUAL(handle->getString("mars.type"), "efic");
        EXPECT_EQUAL(handle->getLong("typeOfProcessedData"), 3);  // control
    });
}

CASE("enfo wave efi: maximum of significant wave height index on the surface, [102, maximum]") {
    run([] {
        auto encoder = metkit::mars2grib::Mars2Grib();
        const auto handle =
            encoder.encode(std::vector<double>(200, 0.5), indexRequest("efi", 132216, 24, 24), climateMisc(51));

        expectModelClimate(handle, 20);
        expectIndexTimeRanges(handle, 2, 0, 24);
        EXPECT_EQUAL(handle->getLong("paramId"), 132216);
        EXPECT_EQUAL(handle->getLong("discipline"), 10);
        EXPECT_EQUAL(handle->getLong("parameterCategory"), 0);
        EXPECT_EQUAL(handle->getLong("parameterNumber"), 3);
        EXPECT_EQUAL(handle->getLong("typeOfFirstFixedSurface"), 1);
    });
}

CASE("enfo sot 90: percentiles {90, 99} and mars.quantile 90-99:100") {
    run([] {
        auto mars = indexRequest("sot", 132144, 96, 24);
        mars.set("quantile", "90-99:100");
        auto misc = climateMisc(51);
        misc.set("efiOrder", 99L);

        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 0.5), mars, misc);

        expectModelClimate(handle, 21);
        expectIndexTimeRanges(handle, 1, 72, 24);
        EXPECT_EQUAL(handle->getLong("numberOfAdditionalParametersForReferencePeriod"), 2);
        EXPECT(handle->getLongArray("scaledValueOfAdditionalParameterForReferencePeriod") ==
               (std::vector<long>{90, 99}));
        EXPECT(handle->getLongArray("scaleFactorOfAdditionalParameterForReferencePeriod") == (std::vector<long>{0, 0}));
        EXPECT_EQUAL(handle->getString("mars.quantile"), "90-99:100");
        EXPECT_EQUAL(handle->getLong("numberOfForecastsInEnsemble"), 51);
        EXPECT_EQUAL(handle->getLong("typeOfProcessedData"), 5);  // control and perturbed
    });
}

CASE("enfo sot 10: percentiles {10, 1} and mars.quantile 1-10:100") {
    run([] {
        auto mars = indexRequest("sot", 132167, 240, 24);
        mars.set("quantile", "1-10:100");

        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 0.5), mars, climateMisc(51));

        expectIndexTimeRanges(handle, 0, 216, 24);
        EXPECT(handle->getLongArray("scaledValueOfAdditionalParameterForReferencePeriod") ==
               (std::vector<long>{10, 1}));
        EXPECT_EQUAL(handle->getString("mars.quantile"), "1-10:100");
    });
}

CASE("sot efiOrder inconsistent with quantile is rejected") {
    auto mars = indexRequest("sot", 132144, 96, 24);
    mars.set("quantile", "90-99:100");
    auto misc = climateMisc(51);
    misc.set("efiOrder", 1L);
    EXPECT_THROWS(metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0.5), mars, misc));
}

CASE("All index parameters preserve their GRIB operations and agree with classification") {
    struct IndexCase {
        long param;
        long inner;
        const char* name;
    };
    const std::array<IndexCase, 11> cases{{
        {132044, 2, "Maximum"},
        {132045, 0, "Average"},
        {132049, 2, "Maximum"},
        {132059, 2, "Maximum"},
        {132144, 1, "Accumulation"},
        {132165, 0, "Average"},
        {132167, 0, "Average"},
        {132201, 2, "Maximum"},
        {132202, 3, "Minimum"},
        {132216, 2, "Maximum"},
        {132228, 1, "Accumulation"},
    }};
    run([&] {
        auto encoder    = metkit::mars2grib::Mars2Grib();
        auto classifier = metkit::mars2grib::Mars2GribClassify();
        for (const auto& test : cases) {
            for (const std::string& type : {"efi", "efic", "sot"}) {
                auto mars = indexRequest(type, test.param, 96, 24);
                if (type == "sot") {
                    mars.set("quantile", "90-99:100");
                }
                const auto misc   = climateMisc(type == "efic" ? 1 : 51);
                const auto handle = encoder.encode(std::vector<double>(200, 0.5), mars, misc);
                expectModelClimate(handle, type == "sot" ? 21 : 20);
                expectIndexTimeRanges(handle, test.inner, 72, 24);
                EXPECT_EQUAL(handle->getLong("paramId"), test.param);
                EXPECT(handle->getLongArray("indicatorOfUnitForTimeRange") == (std::vector<long>{1, 1}));
                EXPECT(handle->getLongArray("typeOfTimeIncrement") == (std::vector<long>{2, 255}));
                EXPECT(handle->getLongArray("indicatorOfUnitForTimeIncrement") == (std::vector<long>{13, 255}));
                EXPECT(handle->getLongArray("timeIncrement") == (std::vector<long>{86400, 0}));
                EXPECT_EQUAL(handle->getLong("yearOfEndOfOverallTimeInterval"), 2026);
                EXPECT_EQUAL(handle->getLong("monthOfEndOfOverallTimeInterval"), 10);
                EXPECT_EQUAL(handle->getLong("dayOfEndOfOverallTimeInterval"), 4);
                EXPECT_EQUAL(handle->getLong("hourOfEndOfOverallTimeInterval"), 0);
                EXPECT_EQUAL(handle->getLong("minuteOfEndOfOverallTimeInterval"), 0);
                EXPECT_EQUAL(handle->getLong("secondOfEndOfOverallTimeInterval"), 0);
                const auto classification = classifier.computeProductTimeSpec(mars, misc);
                EXPECT_EQUAL(classification.classification,
                             std::string("IndexProcessing(86400,") + test.name + "(86400))");
            }
        }
    });
}

CASE("Index statistics preserve explicit increments in medium-range, extended-range and wave streams") {
    run([] {
        auto encoder    = metkit::mars2grib::Mars2Grib();
        auto classifier = metkit::mars2grib::Mars2GribClassify();
        for (const std::string& stream : {"enfo", "eefo", "waef"}) {
            const bool wave = stream == "waef";
            auto mars       = indexRequest("efi", wave ? 132216 : 132045, 168, 24);
            mars.set("stream", stream);
            auto misc = climateMisc(51);
            misc.set("timeIncrementInSeconds", 600L);
            const auto handle = encoder.encode(std::vector<double>(200, 0.5), mars, misc);
            const std::string expectedClassification =
             wave ? "IndexProcessing(86400,Maximum(86400))"
                 : "IndexProcessing(86400,Average(86400))";
            expectIndexTimeRanges(handle, wave ? 2 : 0, 144, 24);
            EXPECT(handle->getLongArray("typeOfTimeIncrement") == (std::vector<long>{2, 2}));
            EXPECT(handle->getLongArray("indicatorOfUnitForTimeIncrement") == (std::vector<long>{13, 13}));
            EXPECT(handle->getLongArray("timeIncrement") == (std::vector<long>{86400, 600}));
            EXPECT_EQUAL(classifier.computeProductTimeSpec(mars, misc).classification,
              expectedClassification);
         }
    });
}

CASE("Index statistics reject from-start, absent-duration and explicit-stattype representations") {
    auto encoder    = metkit::mars2grib::Mars2Grib();
    auto classifier = metkit::mars2grib::Mars2GribClassify();
    const auto misc = climateMisc(51);
    for (const std::string& timespan : {"fs", "none"}) {
        auto mars = indexRequest("efi", 132144, 96, 24);
        mars.set("timespan", timespan);
        EXPECT_THROWS(encoder.encode(std::vector<double>(200, 0.5), mars, misc));
        EXPECT_THROWS(classifier.computeProductTimeSpec(mars, misc));
    }
    auto mars = indexRequest("efi", 132144, 96, 24);
    mars.set("stattype", "daav");
    EXPECT_THROWS(encoder.encode(std::vector<double>(200, 0.5), mars, misc));
    EXPECT_THROWS(classifier.computeProductTimeSpec(mars, misc));
}

CASE("Ordinary accumulation still has one range and unchanged temporal metadata") {
    run([] {
        auto mars = indexRequest("em", 228, 96, 24);
        eckit::LocalConfiguration misc;
        misc.set("numberOfForecastsInEnsemble", 51L);
        auto encoder      = metkit::mars2grib::Mars2Grib();
        auto classifier   = metkit::mars2grib::Mars2GribClassify();
        const auto handle = encoder.encode(std::vector<double>(200, 0.5), mars, misc);
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 12);
        EXPECT_EQUAL(handle->getLong("numberOfTimeRanges"), 1);
        EXPECT(handle->getLongArray("typeOfStatisticalProcessing") == (std::vector<long>{1}));
        EXPECT(handle->getLongArray("lengthOfTimeRange") == (std::vector<long>{24}));
        EXPECT_EQUAL(handle->getLong("forecastTime"), 72);
        EXPECT_EQUAL(classifier.computeProductTimeSpec(mars, misc).classification, "Accumulation(86400)");
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
