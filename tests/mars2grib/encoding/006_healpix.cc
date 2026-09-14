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
#include <exception>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

CASE("H1024") {
    try {
        auto encoder = metkit::mars2grib::Mars2Grib();

        eckit::LocalConfiguration mars;
        mars.set("class", "od");
        mars.set("stream", "oper");
        mars.set("type", "fc");
        mars.set("expver", "test");
        mars.set("grid", "H1024");
        mars.set("packing", "ccsds");
        mars.set("param", 130);
        mars.set("levtype", "pl");
        mars.set("levelist", 1000);
        mars.set("date", 2026'09'14);
        mars.set("time", 00'00);
        mars.set("step", 0);

        std::vector<double> vals(12582912, 237.15);

        const auto handle = encoder.encode(vals, mars);

        // MARS
        EXPECT_EQUAL(handle->getString("gridName"), "H1024");

        // GRIB
        EXPECT_EQUAL(handle->getLong("gridDefinitionTemplateNumber"), 150L);  // HEALPix

        EXPECT_EQUAL(handle->getLong("shapeOfTheEarth"), 6L);  // Spherical Earth with radius = 6371229.0 m
        EXPECT(handle->isMissing("scaleFactorOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaledValueOfRadiusOfSphericalEarth"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMajorAxis"));
        EXPECT(handle->isMissing("scaleFactorOfEarthMinorAxis"));
        EXPECT(handle->isMissing("scaledValueOfEarthMinorAxis"));

        EXPECT_EQUAL(handle->getLong("resolutionAndComponentFlags"), 0L);  // 0000 0000
        EXPECT_EQUAL(handle->getLong("Nside"), 1024L);
        EXPECT_EQUAL(handle->getLong("longitudeOfFirstGridPoint"), 45'000000L);  // Unit 10^-6 degrees (Lo)
        EXPECT_EQUAL(handle->getLong("gridPointPosition"), 4L);                  // Grid points at centre of shapes
        EXPECT_EQUAL(handle->getLong("ordering"), 0L);                           // Ring ordering

        EXPECT_EQUAL(handle->getLong("scanningMode"), 0L);  // 0000 0000

        EXPECT_EQUAL(handle->getLong("numberOfDataPoints"), 12582912);
    }
    catch (const std::exception& e) {
        metkit::mars2grib::utils::exceptions::printExceptionStack(e, eckit::Log::error());
        std::throw_with_nested(
            metkit::mars2grib::utils::exceptions::Mars2GribGenericException("ENCODING TEST FAILED", Here()));
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
