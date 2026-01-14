/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_integer_day.cc
/// @author Metin Cakircali
/// @date   March 2025

#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypeInteger.h"

namespace metkit::mars::test {

using ::eckit::ValueList;
using ::eckit::ValueMap;

//----------------------------------------------------------------------------------------------------------------------

CASE("Test TypeInteger expansion range=[1,100]") {

    ValueMap settings;
    settings["range"] = ValueList{1, 100};
    TypeInteger type("day", settings);
    Type& tday = type;

    // in range

    for (int i = 1; i < 101; ++i) {
        auto num          = std::to_string(i);
        std::string value = num;
        EXPECT(tday.expand(value));
        EXPECT_EQUAL(value, num);
    }

    // out of range
    {
        std::string value = "0";
        EXPECT(!tday.expand(value));
    }
    {
        std::string value = "101";
        EXPECT(!tday.expand(value));
    }
}

//----------------------------------------------------------------------------------------------------------------------

CASE("Test TypeInteger expansion range=[1,1]") {

    ValueMap settings;
    settings["range"] = ValueList{1, 1};
    TypeInteger type("day", settings);
    Type& tday = type;

    {
        std::string value = "1";
        EXPECT(tday.expand(value));
        EXPECT_EQUAL("1", value);
    }

    {
        std::string value = "2";
        EXPECT(!tday.expand(value));
    }
}

//----------------------------------------------------------------------------------------------------------------------

CASE("Test TypeInteger day expansion range=[-1,1]") {

    ValueMap settings;
    settings["range"] = ValueList{-1, 1};
    TypeInteger type("day", settings);
    Type& tday = type;

    {
        std::string value = "-2";
        EXPECT(!tday.expand(value));
    }
    {
        std::string value = "-1";
        EXPECT(tday.expand(value));
        EXPECT_EQUAL("-1", value);
    }
    {
        std::string value = "0";
        EXPECT(tday.expand(value));
        EXPECT_EQUAL("0", value);
    }
    {
        std::string value = "1";
        EXPECT(tday.expand(value));
        EXPECT_EQUAL("1", value);
    }
    {
        std::string value = "2";
        EXPECT(!tday.expand(value));
    }
}

//----------------------------------------------------------------------------------------------------------------------

CASE("Test disseminate day expansion default by/1") {

    std::vector<std::string> expected;
    for (int i = 1; i < 32; ++i) {
        expected.push_back(std::to_string(i));
    }

    const auto* text = R"(disseminate,
  class               = od,
  expver              = 1,
  levtype             = sfc,
  time                = 0,
  stream              = eefo,
  type                = fcmean,
  param               = sd/mn2t6/mx2t6/mtsfr/tcc/stl1/msdr/tprate/msl/rsn/2d/2t/10u/10v,
  step                = 0-168/168-336/336-504/504-672,
  use                 = monday,
  day                 = 1/to/31,
  number              = 0/to/10,
  area                = 90/-180/-90/179.5,
  grid                = .5/.5,
  packing             = simple
)";

    auto request = MarsRequest::parse(text);
    auto days    = request.values("day");
    EXPECT_EQUAL(days, expected);
}

CASE("Test disseminate day expansion") {

    const auto expected = std::vector<std::string>{"1",  "3",  "5",  "7",  "9",  "11", "13", "15",
                                                   "17", "19", "21", "23", "25", "27", "29", "31"};

    const auto* text = R"(disseminate,
  class               = od,
  expver              = 1,
  levtype             = sfc,
  time                = 0,
  stream              = eefo,
  type                = fcmean,
  param               = sd/mn2t6/mx2t6/mtsfr/tcc/stl1/msdr/tprate/msl/rsn/2d/2t/10u/10v,
  step                = 0-168/168-336/336-504/504-672,
  use                 = monday,
  day                 = 1/to/31/by/2,
  number              = 0/to/10,
  area                = 90/-180/-90/179.5,
  grid                = .5/.5,
  packing             = simple
)";

    auto request = MarsRequest::parse(text);
    auto days    = request.values("day");
    EXPECT_EQUAL(days, expected);
}

CASE("Test disseminate day expansion fails outside range") {

    const auto* text = R"(disseminate,
  class               = od,
  expver              = 1,
  levtype             = sfc,
  time                = 0,
  day                 = 1/to/48,
  packing             = simple
)";

    EXPECT_THROWS_AS(MarsRequest::parse(text), eckit::UserError);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
