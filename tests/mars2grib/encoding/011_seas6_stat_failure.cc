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


CASE("SEAS6 Statistics Encoding Failure") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("expver", "jbiq");
    mars.set("class", "rd");
    mars.set("stream", "sfdd");
    mars.set("type", "fc");
    mars.set("grid", "O200");
    mars.set("packing", "ccsds");
    mars.set("param", 235035);
    mars.set("stattype", "momx");
    mars.set("levtype", "sfc");
    mars.set("levelist", 0);
    mars.set("date", 2001'05'01);
    mars.set("time", 00'00);
    // mars.set("step", 744);
    mars.set("fcmonth", 1);
    mars.set("timespan", 24);
    mars.set("number", 0);
    mars.set("system", 0);
    mars.set("method", 1);
    eckit::LocalConfiguration misc;
    // misc.set("timeIncrementInSeconds", 600);

    std::vector<double> vals(167200, 0);

    const auto handle = encoder.encode(vals, mars, misc);

    // MARS
    EXPECT_EQUAL(handle->getLong("step"), 744);
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
