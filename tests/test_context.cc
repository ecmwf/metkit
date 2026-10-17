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

    std::string text1 = "retrieve,class=od,target=abc";

    std::cout << "Parsing first request from text: " << text1 << std::endl;

    metkit::mars::MarsRequest r1 = MarsRequest::parse(text1, true);

    std::cout << "First request: " << r1 << std::endl;

    std::string text2 =
        "retrieve,class=ti,date=20250414,time=12,origin=all,expver=all,type=cf,stream=enfo,"
        "levtype=sfc,param=2t,step=24,expect=any,target=data.reference";
    metkit::mars::MarsRequest r2 = MarsRequest::parse(text2, true);

    Context c{0, eckit::Value{}};

    std::cout << "Initial context: " << c << std::endl;

    EXPECT(c.matches(r1));
    std::cout << "After first match" << std::endl;
    EXPECT(c.matches(r2));
    std::cout << "After second match" << std::endl;

    std::set<std::string> cc{"s2", "ti"};
    c.add(std::make_unique<Include>(MarsLanguage::keyword("class"), cc));
    std::set<std::string> tt{"cf"};
    c.add(std::make_unique<Include>(MarsLanguage::keyword("type"), tt));

    EXPECT(c.matches(r2));
    EXPECT(!c.matches(r1));
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
