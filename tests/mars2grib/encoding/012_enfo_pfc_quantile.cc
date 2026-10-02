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
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace {

// Post-processed quantile forecast (ecPoint) of 6-hourly total precipitation, as in od:0001:enfo
// (MARS param=228,step=126-132 there; mars2mars turns it into param=228228,step=132,timespan=6)
eckit::LocalConfiguration pfcRequest(const std::string& quantile) {
    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", "pfc");
    mars.set("expver", "0001");
    mars.set("domain", "g");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", 228228);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'09'30);
    mars.set("time", 12'00);
    mars.set("step", 132);
    mars.set("timespan", 6);
    mars.set("quantile", quantile);
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

CASE("enfo pfc accumulation is a post-processed quantile forecast (PDT 90)") {
    run([] {
        auto encoder      = metkit::mars2grib::Mars2Grib();
        const auto handle = encoder.encode(std::vector<double>(200, 0.001), pfcRequest("10:100"));

        EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 90);
        EXPECT_EQUAL(handle->getLong("paramId"), 228228);
        EXPECT_EQUAL(handle->getLong("totalNumberOfQuantiles"), 100);
        EXPECT_EQUAL(handle->getLong("quantileValue"), 10);
        EXPECT_EQUAL(handle->getString("mars.quantile"), "10:100");
        EXPECT_EQUAL(handle->getLong("typeOfGeneratingProcess"), 13);
        EXPECT_EQUAL(handle->getLong("typeOfPostProcessing"), 206);
        EXPECT_EQUAL(handle->getLong("inputOriginatingCentre"), 98);
        EXPECT_EQUAL(handle->getLong("inputProcessIdentifier"), 16);
        EXPECT_EQUAL(handle->getLong("typeOfStatisticalProcessing"), 1);
        EXPECT_EQUAL(handle->getLong("lengthOfTimeRange"), 6);
        EXPECT_EQUAL(handle->getString("stepRange"), "126-132");
    });
}

CASE("malformed MARS quantile is rejected") {
    EXPECT_THROWS(metkit::mars2grib::Mars2Grib().encode(std::vector<double>(200, 0.001), pfcRequest("101:100")));
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
