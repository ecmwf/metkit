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
#include <cmath>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/geo/Grid.h"
#include "eckit/geo/Point.h"
#include "eckit/geo/Projection.h"
#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "eckit/types/FloatCompare.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


namespace metkit::mars2grib::test {


static const bool useGridSpec = []() {
    const auto* value = ::getenv("ECCODES_ECKIT_GEO");
    return value != nullptr && std::stol(value) != 0L;
}();


constexpr double EPS = 1e-6;  // [degree] GRIB2 angles are encoded in 10^-6 degree


bool approx(double a, double b) {
    return eckit::types::is_approximately_equal(a, b, EPS);
}


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
        double dx;         // west-east increment [degree]
        double dy;         // south-north increment [degree]
    } tests[]{
        {"1/1", 1., 1.},
        {"0.25/0.25", 0.25, 0.25},
        {"0.1/0.1", 0.1, 0.1},
        {"2/1", 2., 1.},
    };

    for (const auto& [grid, dx, dy] : tests) {
        SECTION(grid) {
            // global, including both poles
            const long Ni = std::lround(360. / dx);
            const long Nj = std::lround(180. / dy) + 1;

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

            EXPECT(handle->getLong("Ni") == Ni);
            EXPECT(handle->getLong("numberOfPointsAlongAParallel") == Ni);
            EXPECT(handle->getLong("Nj") == Nj);
            EXPECT(handle->getLong("numberOfPointsAlongAMeridian") == Nj);
            EXPECT(handle->getLong("basicAngleOfTheInitialProductionDomain") == 0L);
            EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

            EXPECT(approx(handle->getDouble("latitudeOfFirstGridPointInDegrees"), 90.));
            EXPECT(approx(handle->getDouble("longitudeOfFirstGridPointInDegrees"), 0.));
            EXPECT(handle->getLong("resolutionAndComponentFlags") == 48L);  // 0011 0000 (Di and Dj given)
            EXPECT(approx(handle->getDouble("latitudeOfLastGridPointInDegrees"), -90.));
            EXPECT(approx(handle->getDouble("longitudeOfLastGridPointInDegrees"), 360. - dx));
            EXPECT(approx(handle->getDouble("iDirectionIncrementInDegrees"), dx));
            EXPECT(approx(handle->getDouble("jDirectionIncrementInDegrees"), dy));
            EXPECT(handle->getLong("scanningMode") == 0L);  // 0000 0000

            EXPECT(handle->getLong("numberOfDataPoints") == Ni * Nj);
        }
    }
}


CASE("encoding (rotated)") {
    if (!useGridSpec) {
        eckit::Log::warning() << "rotated grids require gridSpec (ECCODES_ECKIT_GEO), test skipped" << std::endl;
        return;
    }

    // the grid of ecCodes' constant_field.grib2 (see also eckit/tests/geo/grid_regular_ll.cc)
    const auto mars = mars_request(R"({"area":[26.65,5.75,-13.25,30.45],"grid":[0.1,0.1],"rotation":[-22,320]})");

    const long Ni = 248;
    const long Nj = 400;

    const std::vector<double> vals(Ni * Nj, 273.15);
    const auto handle = Mars2Grib().encode(vals, mars);
    ASSERT(handle);

    EXPECT(handle->getLong("gridDefinitionTemplateNumber") == 1L);  // Rotated latitude/longitude

    EXPECT(handle->getLong("Ni") == Ni);
    EXPECT(handle->getLong("numberOfPointsAlongAParallel") == Ni);
    EXPECT(handle->getLong("Nj") == Nj);
    EXPECT(handle->getLong("numberOfPointsAlongAMeridian") == Nj);

    // first/last grid points, in the rotated frame
    EXPECT(approx(handle->getDouble("latitudeOfFirstGridPointInDegrees"), 26.65));
    EXPECT(approx(handle->getDouble("longitudeOfFirstGridPointInDegrees"), 5.75));
    EXPECT(approx(handle->getDouble("latitudeOfLastGridPointInDegrees"), -13.25));
    EXPECT(approx(handle->getDouble("longitudeOfLastGridPointInDegrees"), 30.45));
    EXPECT(approx(handle->getDouble("iDirectionIncrementInDegrees"), 0.1));
    EXPECT(approx(handle->getDouble("jDirectionIncrementInDegrees"), 0.1));
    EXPECT(handle->getLong("scanningMode") == 0L);  // 0000 0000

    EXPECT(approx(handle->getDouble("latitudeOfSouthernPoleInDegrees"), -22.));
    EXPECT(approx(handle->getDouble("longitudeOfSouthernPoleInDegrees"), 320.));
    EXPECT(approx(handle->getDouble("angleOfRotationInDegrees"), 0.));

    EXPECT(handle->getLong("numberOfDataPoints") == Ni * Nj);

    // coordinates of the southernmost row (the last, as scanned), from a grid built from the encoded gridSpec
    const std::vector<eckit::geo::PointLonLat> points_ref{
        {-30.37923437901262, 54.30167983367708}, {-30.21461696695217, 54.28605285392668},
        {-30.05013625049183, 54.27016346802356}, {-29.88579431820473, 54.25401202651201},
        {-29.72159324673793, 54.23759888507955}, {-29.55753510066738, 54.22092440451927},
        {-29.39362193235515, 54.20398895069178}, {-29.22985578180936, 54.18679289448672},
        {-29.06623867654653, 54.16933661178392}, {-28.90277263145643, 54.15162048341411},
    };

    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::make_from_string(handle->getString("gridSpec")));
    ASSERT(grid);

    EXPECT(grid->projection().type() == "rotation");
    EXPECT(grid->size() == static_cast<size_t>(Ni * Nj));

    const auto points = grid->to_points();
    for (size_t i = 0; i < points_ref.size(); ++i) {
        EXPECT(eckit::geo::points_equal(points[(Nj - 1) * Ni + i], points_ref[i], EPS));
    }
}


}  // namespace metkit::mars2grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
