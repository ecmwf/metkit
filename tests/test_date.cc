/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_date.cc
/// @author Emanuele Danovaro
/// @date   March 2025

#include <string>
#include <vector>

#include "eckit/testing/Test.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/TypeDate.h"

namespace metkit::mars::test {

using ::eckit::BadValue;
using ::eckit::Value;

//-----------------------------------------------------------------------------

void assertTypeExpansion(const std::string& name, std::vector<std::string> values,
                         const std::vector<std::string>& expected) {
    static MarsLanguage language("retrieve");
    language.type(name)->expand(DummyContext(), values);
    EXPECT_EQUAL(expected, values);
}

CASE("Test TypeDate expansions") {

    TypeDate tdate("date", Value());
    Type& td(tdate);
    DummyContext ctx;

    assertTypeExpansion("date", {"20140506"}, {"20140506"});
    assertTypeExpansion("date", {"2014-05-06"}, {"20140506"});
    assertTypeExpansion("date", {"20140506", "20140507"}, {"20140506", "20140507"});
    assertTypeExpansion("date", {"20140506", "to", "20140506"}, {"20140506"});
    assertTypeExpansion("date", {"20140506", "to", "20140507"}, {"20140506", "20140507"});
    assertTypeExpansion("date", {"20140506", "to", "20140508"}, {"20140506", "20140507", "20140508"});
    assertTypeExpansion("date", {"20140504", "20140506", "to", "20140508"},
                        {"20140504", "20140506", "20140507", "20140508"});
    assertTypeExpansion("date", {"2"}, {"2"});
    assertTypeExpansion("date", {"jan"}, {"1"});
    assertTypeExpansion("date", {"1-01"}, {"101"});
    assertTypeExpansion("date", {"jan-01"}, {"101"});
    assertTypeExpansion("date", {"feb-23"}, {"223"});

    {
        std::string value = "20141506";
        EXPECT_THROWS(td.expand(ctx, value));  // throws BadDate that is not exported
    }
    {
        std::string value = "abc";
        EXPECT_THROWS_AS(td.expand(ctx, value), BadValue);
    }
    {
        std::string value = "abc-01";
        EXPECT_THROWS_AS(td.expand(ctx, value), BadValue);
    }
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
