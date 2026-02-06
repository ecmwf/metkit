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

namespace metkit::test {

CASE("mars2grib_api") {
    auto encoder = mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("origin", "ecmf");
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "0001");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "hl");
    mars.set("levelist", 2);
    mars.set("date", 2026'02'05);
    mars.set("time", 00'00'00);
    mars.set("step", 0);

    std::vector<double> vals(200, 237.15);

    encoder.encode(vals, mars);
}

}  // namespace metkit::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
