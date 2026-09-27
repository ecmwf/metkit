/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


namespace metkit::mars2grib::test {


static const bool useGridSpec = []() {
    const auto* value = ::getenv("ECCODES_ECKIT_GEO");
    return value != nullptr && std::stol(value) != 0L;
}();


eckit::LocalConfiguration mars_request(const std::string& grid) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", grid);
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'10);
    mars.set("time", 00'00);
    mars.set("step", 0);
    return mars;
}


CASE("encoding") {
    struct test_t {
        std::string grid;  // MARS grid (west-east/south-north increments)
        long Di;           // west-east increment [10^-6 degrees]
        long Dj;           // south-north increment [10^-6 degrees]
    } tests[]{
        {"1/1", 1'000000, 1'000000},
        {"0.25/0.25", 250000, 250000},
        {"0.1/0.1", 100000, 100000},
        {"2/1", 2'000000, 1'000000},
    };

    for (const auto& [grid, Di, Dj] : tests) {
        SECTION(grid) {
            // global, including both poles
            const long Ni = 360'000000 / Di;
            const long Nj = 180'000000 / Dj + 1;

            const std::vector<double> vals(Ni * Nj, 273.15);
            const auto handle = Mars2Grib().encode(vals, mars_request(grid));
            ASSERT(handle);

            if (useGridSpec) {
                auto increments = grid;
                std::replace(increments.begin(), increments.end(), '/', ',');
                EXPECT(handle->getString("gridSpec") == R"({"grid":[)" + increments + "]}");
            }

            EXPECT(handle->getLong("gridDefinitionTemplateNumber") == 0L);  // Latitude/longitude

            EXPECT(handle->getLong("shapeOfTheEarth") == 6L);  // Spherical Earth with radius = 6371229.0 m
            EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
            EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
            EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
            EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
            EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
            EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

            EXPECT(handle->getLong("numberOfPointsAlongAParallel") == Ni);
            EXPECT(handle->getLong("numberOfPointsAlongAMeridian") == Nj);
            EXPECT(handle->getLong("basicAngleOfTheInitialProductionDomain") == 0L);
            EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

            EXPECT(handle->getLong("latitudeOfFirstGridPoint") == 90'000000L);
            EXPECT(handle->getLong("longitudeOfFirstGridPoint") == 0L);
            EXPECT(handle->getLong("resolutionAndComponentFlags") == 48L);  // 0011 0000 (Di and Dj given)
            EXPECT(handle->getLong("latitudeOfLastGridPoint") == -90'000000L);
            EXPECT(handle->getLong("longitudeOfLastGridPoint") == 360'000000L - Di);
            EXPECT(handle->getLong("iDirectionIncrement") == Di);
            EXPECT(handle->getLong("jDirectionIncrement") == Dj);
            EXPECT(handle->getLong("scanningMode") == 0L);  // 0000 0000

            EXPECT(handle->getLong("numberOfDataPoints") == Ni * Nj);
        }
    }
}


}  // namespace metkit::mars2grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
