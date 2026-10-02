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

// Ensemble probabilities of od:0001:enfo after mars2mars (step range -> step + timespan)
eckit::LocalConfiguration epRequest(long param, const std::string& levtype, long step, const std::string& timespan) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", "ep");
    mars.set("expver", "0001");
    mars.set("domain", "g");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", levtype);
    if (levtype == "pl") {
        mars.set("levelist", 850);
    }
    mars.set("date", 2026'09'30);
    mars.set("time", 12'00);
    mars.set("step", step);
    mars.set("timespan", timespan);
    return mars;
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

CASE("tp probability over 24h (PDT 9), probability keys from the paramId") {
    run([] {
        const auto handle =
            metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0.5), epRequest(131060, "sfc", 24, "24h"));
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 9);
        EXPECT_EQUAL(handle->getLong("paramId"), 131060);
        EXPECT_EQUAL(handle->getLong("probabilityType"), 3);
        EXPECT_EQUAL(handle->getLong("typeOfStatisticalProcessing"), 1);
        EXPECT_EQUAL(handle->getLong("lengthOfTimeRange"), 24);
        EXPECT_EQUAL(handle->getLong("forecastTime"), 0);
    });
}

CASE("10m wind speed probability: instant (PDT 5), time-window maximum (PDT 9)") {
    run([] {
        auto encoder        = metkit::mars2grib::Mars2Grib();
        const auto instant  = encoder.encode(std::vector<double>(200, 0.5), epRequest(131068, "sfc", 120, "none"));
        const auto inWindow = encoder.encode(std::vector<double>(200, 0.5), epRequest(131068, "sfc", 168, "48h"));
        EXPECT_EQUAL(instant->getLong("productDefinitionTemplateNumber"), 5);
        EXPECT_EQUAL(instant->getLong("paramId"), 131068);
        EXPECT_EQUAL(instant->getLong("forecastTime"), 120);
        EXPECT_EQUAL(inWindow->getLong("productDefinitionTemplateNumber"), 9);
        EXPECT_EQUAL(inWindow->getLong("paramId"), 131068);
        EXPECT_EQUAL(inWindow->getLong("typeOfStatisticalProcessing"), 2);
        EXPECT_EQUAL(inWindow->getLong("forecastTime"), 120);
        EXPECT_EQUAL(inWindow->getLong("lengthOfTimeRange"), 48);
    });
}

CASE("strike probability over 48h (PDT 122)") {
    run([] {
        const auto handle =
            metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0.5), epRequest(131089, "sfc", 216, "48h"));
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 122);
        EXPECT_EQUAL(handle->getLong("paramId"), 131089);
        EXPECT_EQUAL(handle->getLong("probabilityType"), 9);
        EXPECT_EQUAL(handle->getLong("forecastTime"), 168);
        EXPECT_EQUAL(handle->getLong("lengthOfTimeRange"), 48);
    });
}

CASE("probability of a standardised temperature anomaly, instant (PDT 131)") {
    run([] {
        const auto handle =
            metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0.5), epRequest(133093, "pl", 120, "none"));
        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 131);
        EXPECT_EQUAL(handle->getLong("paramId"), 133093);
        EXPECT_EQUAL(handle->getLong("typeOfRelationToReferenceDataset"), 1);
        EXPECT_EQUAL(handle->getLong("probabilityType"), 3);
    });
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
