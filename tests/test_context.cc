// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   test_step.cc
/// @author Emanuele Danovaro
/// @date   March 2025

#include <memory>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"

namespace metkit::mars::test {

using ::eckit::BadValue;

//-----------------------------------------------------------------------------

CASE("Context match") {

    Context c;
    std::set<std::string> cc{"s2", "ti"};
    c.add(std::make_unique<Include>("class", cc));
    std::set<std::string> tt{"cf"};
    c.add(std::make_unique<Include>("type", tt));

    std::string text =
        "retrieve,  "
        "class=ti,date=20250414,time=12,origin=all,expver=all,type=cf,stream=enfo,levtype=sfc,param=2t,step=24,expect="
        "any,target=data.reference";

    metkit::mars::MarsRequest r = MarsRequest::parse(text, true);

    EXPECT(c.matches(r));
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
