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
#include "metkit/mars2grib/api/Mars2Grib.h"

static const bool useGridSpec = []() {
    const auto* value = ::getenv("ECCODES_ECKIT_GEO");
    return value != nullptr && std::stol(value) != 0L;
}();

constexpr double EPS = 1e-6;  // [degree] GRIB2 angles are encoded in 10^-6 degree

bool approx(double a, double b) {
    return eckit::types::is_approximately_equal(a, b, EPS);
}

CASE("N32") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "N32");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(6114, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("gridName"), "N32");

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));           // Missing for reduced gaussian grids (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 64L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 87'863799L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -87'863799L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 357'187500L);  // Unit 10^-6 degrees (Lo2)
    EXPECT(handle->isMissing("iDirectionIncrement"));                        // Missing for reduced gaussian grids (Di)
    EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 32);  // (N)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 6114);
}

CASE("O32") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "O32");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(5248, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("gridName"), "O32");

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));           // Missing for reduced gaussian grids (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 64L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 87'863799L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -87'863799L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 357'500000L);  // Unit 10^-6 degrees (Lo2)
    EXPECT(handle->isMissing("iDirectionIncrement"));                        // Missing for reduced gaussian grids (Di)
    EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 32);  // (N)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 5248);
}

CASE("N1280") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "N1280");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(8505906, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("gridName"), "N1280");

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Gaussian latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));             // Missing for reduced gaussian grids (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 2560L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 89'946188L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -89'946188L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'929688L);  // Unit 10^-6 degrees (Lo2)
    EXPECT(handle->isMissing("iDirectionIncrement"));                        // Missing for reduced gaussian grids (Di)
    EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 1280);  // (N)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 8505906);
}

CASE("O1280") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", "O1280");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(6599680, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("gridName"), "O1280");

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 40L);  // Gaussian latitude/longitude

    EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
    EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
    EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
    EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

    EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));             // Missing for reduced gaussian grids (Ni)
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 2560L);  // (Nj)
    EXPECT_EQUAL(handle->getLong("basicAngleOfTheInitialProductionDomain"), 0L);
    EXPECT(handle->isMissing("subdivisionsOfBasicAngle"));

    EXPECT_EQUAL(handle->getLong("latitudeOfFirstGridPoint"), 89'946188L);   // Unit 10^-6 degrees (La1)
    EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 0'000000L);   // Unit 10^-6 degrees (Lo1)
    EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);        // 0000 0000  Di NOT given
    EXPECT_EQUAL(handle->getLong("latitudeOfLastGridPoint"), -89'946188L);   // Unit 10^-6 degrees (La2)
    EXPECT_EQUAL(handle->getLong("longitudeOfLastGridPoint"), 359'929907L);  // Unit 10^-6 degrees (Lo2)
    EXPECT(handle->isMissing("iDirectionIncrement"));                        // Missing for reduced gaussian grids (Di)
    EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 1280);  // (N)

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 6599680);
}

CASE("O80 (rotated)") {
    if (!useGridSpec) {
        eckit::Log::warning() << "rotated grids require gridSpec (ECCODES_ECKIT_GEO), test skipped" << std::endl;
        return;
    }

    auto encoder = metkit::mars2grib::Mars2Grib();

    // the grid of ecCodes' gridType=reduced_rotated_gg.grib (third message)
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "oper");
    mars.set("type", "fc");
    mars.set("expver", "test");
    mars.set("grid", R"({"grid":"O80","rotation":[30,30]})");
    mars.set("packing", "ccsds");
    mars.set("param", 130);
    mars.set("levtype", "pl");
    mars.set("levelist", 1000);
    mars.set("date", 2026'09'14);
    mars.set("time", 00'00);
    mars.set("step", 0);

    std::vector<double> vals(28480, 237.15);

    const auto handle = encoder.encode(vals, mars);

    // GRIB
    EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 41L);  // Rotated Gaussian latitude/longitude

    EXPECT(handle->isMissing("Ni"));  // Missing for reduced gaussian grids
    EXPECT(handle->isMissing("numberOfPointsAlongAParallel"));
    EXPECT_EQUAL(handle->getLong("Nj"), 160L);
    EXPECT_EQUAL(handle->getLong("numberOfPointsAlongAMeridian"), 160L);
    EXPECT_EQUAL(handle->getLong("N"), 80);
    EXPECT_EQUAL(handle->getLong("numberOfParallelsBetweenAPoleAndTheEquator"), 80);

    EXPECT(approx(handle->getDouble("latitudeOfFirstGridPointInDegrees"), 89.141519));
    EXPECT(approx(handle->getDouble("longitudeOfFirstGridPointInDegrees"), 0.));
    EXPECT(approx(handle->getDouble("latitudeOfLastGridPointInDegrees"), -89.141519));
    EXPECT(approx(handle->getDouble("longitudeOfLastGridPointInDegrees"), 358.928571));
    EXPECT(handle->isMissing("iDirectionIncrement"));  // Missing for reduced gaussian grids

    EXPECT(approx(handle->getDouble("latitudeOfSouthernPoleInDegrees"), 30.));
    EXPECT(approx(handle->getDouble("longitudeOfSouthernPoleInDegrees"), 30.));
    EXPECT(approx(handle->getDouble("angleOfRotationInDegrees"), 0.));

    EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

    EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 28480);

    // coordinates of the first points, from a grid built from the encoded gridSpec
    const std::vector<eckit::geo::PointLonLat> points_ref{
        {-150.00000000000000, -29.14151942646109}, {-150.30384489745967, -29.18318764253491},
        {-150.57863834893357, -29.30420953577266}, {-150.79791950179472, -29.49299204833164},
        {-150.94024317792739, -29.73137429450929}, {-150.99126325558376, -29.99628694451165},
        {-150.94528301220487, -30.26190855703454}, {-150.80607488769962, -30.50214705912438},
        {-150.58679467788403, -30.69322436612787}, {-150.30888625761696, -30.81610305894176},
    };

    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::make_from_string(handle->getString("gridSpec")));
    ASSERT(grid);

    EXPECT(grid->projection().type() == "rotation");
    EXPECT(grid->size() == 28480);

    const auto points = grid->to_points();
    for (size_t i = 0; i < points_ref.size(); ++i) {
        EXPECT(eckit::geo::points_equal(points[i], points_ref[i], EPS));
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
