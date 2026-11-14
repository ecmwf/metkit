/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file mars2mars-test-04.cc
/// @brief The shift-of-tails percentile is converted from MARS `number` to `quantile`.

#include <exception>
#include <stdexcept>
#include <string>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/mars2mars/api/Mars2Mars.h"

namespace {

eckit::LocalConfiguration request(const std::string& type, long number) {
    eckit::LocalConfiguration mars;
    mars.set("class", "od");
    mars.set("stream", "enfo");
    mars.set("type", type);
    mars.set("expver", "0001");
    mars.set("date", 20260930L);
    mars.set("time", 0L);
    mars.set("levtype", "sfc");
    mars.set("param", 132144L);
    mars.set("step", "72-96");
    mars.set("number", number);
    return mars;
}

eckit::LocalConfiguration converted(const eckit::LocalConfiguration& in) {
    try {
        return metkit::mars2mars::Mars2Mars{}.convert(in).mars;
    }
    catch (const std::exception& e) {
        throw std::runtime_error(std::string("Test failed with exception: ") + e.what());
    }
}

}  // namespace

CASE("sot 90 becomes quantile 90-99:100") {
    const auto out = converted(request("sot", 90));
    EXPECT_EQUAL(out.getString("quantile"), "90-99:100");
    EXPECT(!out.has("number"));
}

CASE("sot 10 becomes quantile 1-10:100") {
    const auto out = converted(request("sot", 10));
    EXPECT_EQUAL(out.getString("quantile"), "1-10:100");
    EXPECT(!out.has("number"));
}

CASE("other sot percentiles are rejected") {
    EXPECT_THROWS(metkit::mars2mars::Mars2Mars{}.convert(request("sot", 50)));
}

CASE("number of other types is unchanged") {
    const auto out = converted(request("pf", 7));
    EXPECT_EQUAL(out.getLong("number"), 7);
    EXPECT(!out.has("quantile"));
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
