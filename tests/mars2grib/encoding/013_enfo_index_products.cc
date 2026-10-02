/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <exception>
#include <string>
#include <vector>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
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

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
