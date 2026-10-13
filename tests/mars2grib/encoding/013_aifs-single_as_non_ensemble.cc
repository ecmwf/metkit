/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


std::vector<double> getVals() {
    return std::vector<double>(542080, 0.0);
}

eckit::LocalConfiguration getMars() {
    eckit::LocalConfiguration mars;
    mars.set("class", "ai");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "v300");
    mars.set("model", "aifs-single");
    mars.set("grid", "N320");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 850);
    mars.set("date", 2026'09'30);
    mars.set("time", 00'00);
    mars.set("step", 42);
    mars.set("timespan", "none");
    return mars;
}

eckit::LocalConfiguration getMisc() {
    eckit::LocalConfiguration misc;
    misc.set("generatingProcessIdentifier", 5);
    misc.set("bitmapPresent", false);
    return misc;
}

CASE("encoding") {
    auto encoder      = metkit::mars2grib::Mars2Grib();
    const auto handle = encoder.encode(getVals(), getMars(), getMisc());

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 0);  // Point in time analysis or forecast
    EXPECT_EQUAL(handle->getLong("typeOfGeneratingProcess"), 2);          // Forecast
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
