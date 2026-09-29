/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);
    return mars;
}


CASE("encoding") {
    struct test_t {
        std::string grid;  // MARS grid
        long N;            // number of parallels between a pole and the equator
        double lat1;       // latitude of the first grid point (northernmost Gaussian latitude) [degree]
    } tests[]{
        {"F32", 32, 87.863799},
        {"F1280", 1280, 89.946188},
    };

    for (const auto& [grid, N, lat1] : tests) {
        SECTION(grid) {
            // global, 4N points along a parallel
            const long Ni   = 4 * N;
            const long Nj   = 2 * N;
            const double dx = 90. / N;

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

            EXPECT(handle->getLong("Ni") == Ni);
            EXPECT(handle->getLong("numberOfPointsAlongAParallel") == Ni);
            EXPECT(handle->getLong("Nj") == Nj);
            EXPECT(handle->getLong("numberOfPointsAlongAMeridian") == Nj);
            EXPECT(handle->getLong("N") == N);
            EXPECT(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator") == N);
            EXPECT(handle->getLong("basicAngleOfTheInitialProductionDomain") == 0L);
            EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

            // ECC-2336: i/jDirectionIncrementGiven flags are currently not correctly encoded in eccodes with gridSpec
            // EXPECT(handle->getLong("resolutionAndComponentFlags") == 32L);  // 0010 0000 (Di given)
            EXPECT(approx(handle->getDouble("latitudeOfFirstGridPointInDegrees"), lat1));
            EXPECT(approx(handle->getDouble("longitudeOfFirstGridPointInDegrees"), 0.));
            EXPECT(approx(handle->getDouble("latitudeOfLastGridPointInDegrees"), -lat1));
            EXPECT(approx(handle->getDouble("longitudeOfLastGridPointInDegrees"), 360. - dx));
            EXPECT(approx(handle->getDouble("iDirectionIncrementInDegrees"), dx));
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

    // the grid of ecCodes' gridType=rotated_gg.grib (first message)
    const auto mars = mars_request(R"({"grid":"F48","rotation":[30,30]})");

    const long Ni = 192;
    const long Nj = 96;

    const std::vector<double> vals(Ni * Nj, 273.15);
    const auto handle = Mars2Grib().encode(vals, mars);
    ASSERT(handle);

    EXPECT(handle->getLong("gridDefinitionTemplateNumber") == 41L);  // Rotated Gaussian latitude/longitude

    EXPECT(handle->getLong("Ni") == Ni);
    EXPECT(handle->getLong("numberOfPointsAlongAParallel") == Ni);
    EXPECT(handle->getLong("Nj") == Nj);
    EXPECT(handle->getLong("numberOfPointsAlongAMeridian") == Nj);
    EXPECT(handle->getLong("N") == 48L);
    EXPECT(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator") == 48L);

    // first/last grid points, in the rotated frame
    EXPECT(approx(handle->getDouble("latitudeOfFirstGridPointInDegrees"), 88.572169));
    EXPECT(approx(handle->getDouble("longitudeOfFirstGridPointInDegrees"), 0.));
    EXPECT(approx(handle->getDouble("latitudeOfLastGridPointInDegrees"), -88.572169));
    EXPECT(approx(handle->getDouble("longitudeOfLastGridPointInDegrees"), 358.125));
    EXPECT(approx(handle->getDouble("iDirectionIncrementInDegrees"), 1.875));
    EXPECT(handle->getLong("scanningMode") == 0L);  // 0000 0000

    EXPECT(approx(handle->getDouble("latitudeOfSouthernPoleInDegrees"), 30.));
    EXPECT(approx(handle->getDouble("longitudeOfSouthernPoleInDegrees"), 30.));
    EXPECT(approx(handle->getDouble("angleOfRotationInDegrees"), 0.));

    EXPECT(handle->getLong("numberOfDataPoints") == Ni * Nj);

    // coordinates of the first points, from a grid built from the encoded gridSpec
    const std::vector<eckit::geo::PointLonLat> points_ref{
        {-150.00000000000000, -28.57216851400726}, {-150.05319064425672, -28.57292230625801},
        {-150.10632666115495, -28.57518290821670}, {-150.15935347266884, -28.57894799620847},
        {-150.21221659941676, -28.58421369979395}, {-150.26486170993135, -28.59097460529629},
        {-150.31723466986631, -28.59922376073626}, {-150.36928159111906, -28.60895268217335},
        {-150.42094888084813, -28.62015136144922}, {-150.47218329036292, -28.63280827532991},
    };

    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::make_from_string(handle->getString("gridSpec")));
    ASSERT(grid);

    EXPECT(grid->projection().type() == "rotation");
    EXPECT(grid->size() == static_cast<size_t>(Ni * Nj));

    const auto points = grid->to_points();
    for (size_t i = 0; i < points_ref.size(); ++i) {
        EXPECT(eckit::geo::points_equal(points[i], points_ref[i], EPS));
    }
}


}  // namespace metkit::mars2grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
