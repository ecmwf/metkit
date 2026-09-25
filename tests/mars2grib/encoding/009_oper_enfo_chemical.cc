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


eckit::LocalConfiguration getMars(const std::string& stream, const std::string& type, long param,
                                  const std::string& timespan = "none") {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", stream);
    mars.set("type", type);
    mars.set("expver", "test");
    mars.set("grid", "N200");
    mars.set("packing", "ccsds");
    mars.set("param", param);
    mars.set("levtype", "sfc");
    mars.set("date", 2026'09'25);
    mars.set("time", 00'00);
    mars.set("step", 42);
    mars.set("timespan", timespan);
    if (type == "pf") {
        mars.set("number", 42);
    }
    return mars;
}

CASE("stream=oper,type=fc,param=228080") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("oper", "fc", 228080, "fs");
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "fc");
    EXPECT_EQUAL(handle->getLong("paramId"), 228080);
    EXPECT_EQUAL(handle->getString("timespan"), "fs");
    EXPECT_EQUAL(handle->getLong("number"), 0);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 43);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=oper,type=fc,param=228081") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("oper", "fc", 228081, "fs");
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "fc");
    EXPECT_EQUAL(handle->getLong("paramId"), 228081);
    EXPECT_EQUAL(handle->getString("timespan"), "fs");
    EXPECT_EQUAL(handle->getLong("number"), 0);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 43);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=oper,type=fc,param=228082") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("oper", "fc", 228082, "fs");
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "fc");
    EXPECT_EQUAL(handle->getLong("paramId"), 228082);
    EXPECT_EQUAL(handle->getString("timespan"), "fs");
    EXPECT_EQUAL(handle->getLong("number"), 0);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 43);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=oper,type=fc,param=228083") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("oper", "fc", 228083);
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "fc");
    EXPECT_EQUAL(handle->getLong("paramId"), 228083);
    EXPECT_EQUAL(handle->getString("timespan"), "none");
    EXPECT_EQUAL(handle->getLong("number"), 0);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 41);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=oper,type=fc,param=228084") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("oper", "fc", 228084);
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "fc");
    EXPECT_EQUAL(handle->getLong("paramId"), 228084);
    EXPECT_EQUAL(handle->getString("timespan"), "none");
    EXPECT_EQUAL(handle->getLong("number"), 0);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 41);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=oper,type=fc,param=228085") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("oper", "fc", 228085);
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "fc");
    EXPECT_EQUAL(handle->getLong("paramId"), 228085);
    EXPECT_EQUAL(handle->getString("timespan"), "none");
    EXPECT_EQUAL(handle->getLong("number"), 0);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 41);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}


CASE("stream=enfo,type=pf,param=228080") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("enfo", "pf", 228080, "fs");
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "pf");
    EXPECT_EQUAL(handle->getLong("paramId"), 228080);
    EXPECT_EQUAL(handle->getString("timespan"), "fs");
    EXPECT_EQUAL(handle->getLong("number"), 42);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 43);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=enfo,type=pf,param=228081") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("enfo", "pf", 228081, "fs");
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "pf");
    EXPECT_EQUAL(handle->getLong("paramId"), 228081);
    EXPECT_EQUAL(handle->getString("timespan"), "fs");
    EXPECT_EQUAL(handle->getLong("number"), 42);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 43);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=enfo,type=pf,param=228082") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("enfo", "pf", 228082, "fs");
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "pf");
    EXPECT_EQUAL(handle->getLong("paramId"), 228082);
    EXPECT_EQUAL(handle->getString("timespan"), "fs");
    EXPECT_EQUAL(handle->getLong("number"), 42);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 43);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=enfo,type=pf,param=228083") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("enfo", "pf", 228083);
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "pf");
    EXPECT_EQUAL(handle->getLong("paramId"), 228083);
    EXPECT_EQUAL(handle->getString("timespan"), "none");
    EXPECT_EQUAL(handle->getLong("number"), 42);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 41);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=enfo,type=pf,param=228084") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("enfo", "pf", 228084);
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "pf");
    EXPECT_EQUAL(handle->getLong("paramId"), 228084);
    EXPECT_EQUAL(handle->getString("timespan"), "none");
    EXPECT_EQUAL(handle->getLong("number"), 42);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 41);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}

CASE("stream=enfo,type=pf,param=228085") {
    auto encoder = metkit::mars2grib::Mars2Grib();

    const auto mars = getMars("enfo", "pf", 228085);
    std::vector<double> vals(200, 0);

    const auto handle = encoder.encode(vals, mars);

    // MARS
    EXPECT_EQUAL(handle->getString("type"), "pf");
    EXPECT_EQUAL(handle->getLong("paramId"), 228085);
    EXPECT_EQUAL(handle->getString("timespan"), "none");
    EXPECT_EQUAL(handle->getLong("number"), 42);

    // GRIB
    EXPECT_EQUAL(handle->getLong("productDefinitionTemplateNumber"), 41);
    EXPECT_EQUAL(handle->getLong("constituentType"), 3);
}


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
