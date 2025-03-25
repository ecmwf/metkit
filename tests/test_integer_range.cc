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

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"
#include "eckit/value/Content.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypeInteger.h"
#include "metkit/mars/TypesFactory.h"

using namespace eckit;
using namespace eckit::testing;

namespace metkit::mars::test {

const DummyContext ctx;

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
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, true);
        EXPECT_EQUAL(value, num);
    }

    // out of range

    {
        std::string value = "0";
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, false);
    }

    {
        std::string value = "101";
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, false);
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
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, true);
    }

    {
        std::string value = "2";
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, false);
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
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, false);
    }

    {
        std::string value = "-1";
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, true);
    }

    {
        std::string value = "0";
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, true);
    }

    {
        std::string value = "1";
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, true);
    }

    {
        std::string value = "2";
        auto ret          = tday.expand(ctx, value);
        EXPECT_EQUAL(ret, false);
    }
}

//----------------------------------------------------------------------------------------------------------------------

CASE("Test disseminate day expansion default by/2") {

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
    return run_tests(argc, argv);
}
