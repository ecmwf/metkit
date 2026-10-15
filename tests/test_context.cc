// SPDX-FileCopyrightText: 2025- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   test_step.cc
/// @author Emanuele Danovaro
/// @date   March 2025

#include <memory>
#include <set>
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

CASE("Context match with alternatives") {
    MarsRequest r("retrieve");
    r.setValue("param", "228032|32");

    auto include = [](const std::set<std::string>& vals) {
        Context c;
        c.add(std::make_unique<Include>("param", vals));
        return c;
    };
    auto exclude = [](const std::set<std::string>& vals) {
        Context c;
        c.add(std::make_unique<Exclude>("param", vals));
        return c;
    };

    // Include: any alternative is enough
    EXPECT(include({"32"}).matches(r));
    EXPECT(include({"228032"}).matches(r));
    EXPECT(!include({"167"}).matches(r));

    // Exclude: no alternative may be listed
    EXPECT(!exclude({"32"}).matches(r));
    EXPECT(!exclude({"228032"}).matches(r));
    EXPECT(exclude({"167"}).matches(r));
}

CASE("Context match with multiple alternatives") {
    const std::set<std::string> vals{"32", "167"};

    auto includes = [](const std::set<std::string>& values, const std::string& param) {
        MarsRequest r("retrieve");
        r.setValue("param", param);
        Context c;
        c.add(std::make_unique<Include>("param", values));
        return c.matches(r);
    };
    auto excludes = [](const std::set<std::string>& values, const std::string& param) {
        MarsRequest r("retrieve");
        r.setValue("param", param);
        Context c;
        c.add(std::make_unique<Exclude>("param", values));
        return c.matches(r);
    };

    EXPECT(includes(vals, "32"));
    EXPECT(includes(vals, "228032|32"));
    EXPECT(includes(vals, "228032|167|3062"));
    EXPECT(!includes(vals, "228032"));
    EXPECT(!includes(vals, "228032|3062"));
    EXPECT(!includes(vals, "|"));
    EXPECT(!includes({}, "32|167"));

    EXPECT(!excludes(vals, "32"));
    EXPECT(!excludes(vals, "228032|32"));
    EXPECT(!excludes(vals, "228032|167|3062"));
    EXPECT(excludes(vals, "228032"));
    EXPECT(excludes(vals, "228032|3062"));
    EXPECT(excludes(vals, "|"));
    EXPECT(excludes({}, "32|167"));
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
