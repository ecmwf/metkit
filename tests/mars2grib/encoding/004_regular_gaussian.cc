/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <cmath>
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
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);
    return mars;
}


CASE("encoding") {
    struct test_t {
        std::string grid;  // MARS grid
        long N;            // number of parallels between a pole and the equator
        long La1;          // latitude of the first grid point (northernmost Gaussian latitude) [10^-6 degrees]
    } tests[]{
        {"F32", 32, 87'863799},
        {"F1280", 1280, 89'946188},
    };

    for (const auto& [grid, N, La1] : tests) {
        SECTION(grid) {
            // global, 4N points along a parallel
            const long Ni = 4 * N;
            const long Nj = 2 * N;
            const long Di = std::lround(90'000000. / N);

            const std::vector<double> vals(Ni * Nj, 273.15);
            const auto handle = Mars2Grib().encode(vals, mars_request(grid));
            ASSERT(handle);

            EXPECT(handle->getString("gridName") == grid);
            if (useGridSpec) {
                EXPECT(handle->getString("gridSpec") == R"({"grid":")" + grid + R"("})");
            }

            EXPECT(handle->getLong("gridDefinitionTemplateNumber") == 40L);  // Gaussian latitude/longitude

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

            // ECC-2336: i/jDirectionIncrementGiven flags are currently not correctly encoded in eccodes with gridSpec
            // EXPECT(handle->getLong("resolutionAndComponentFlags") == 32L);  // 0010 0000 (Di given)
            EXPECT(handle->getLong("latitudeOfFirstGridPoint") == La1);
            EXPECT(handle->getLong("longitudeOfFirstGridPoint") == 0L);
            EXPECT(handle->getLong("latitudeOfLastGridPoint") == -La1);
            EXPECT(handle->getLong("longitudeOfLastGridPoint") == std::lround(360'000000. - 90'000000. / N));
            EXPECT(handle->getLong("iDirectionIncrement") == Di);
            EXPECT(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator") == N);
            EXPECT(handle->getLong("scanningMode") == 0L);  // 0000 0000

            EXPECT(handle->getLong("numberOfDataPoints") == Ni * Nj);
        }
    }
}


}  // namespace metkit::mars2grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
