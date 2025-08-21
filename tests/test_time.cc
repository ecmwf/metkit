/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_time.cc
/// @author Simon Smart
/// @author Emanuele Danovaro
/// @date   April 2020

#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypeTime.h"

namespace metkit::mars::test {

using ::eckit::BadTime;
using ::eckit::BadValue;
using ::eckit::Value;

//-----------------------------------------------------------------------------

void assertTypeExpansion(const std::string& name, std::vector<std::string> values,
                         const std::vector<std::string>& expected) {
    static MarsLanguage language("retrieve");
    language.type(name)->expand(values);
    EXPECT_EQUAL(values.size(), expected.size());
    EXPECT_EQUAL(expected, values);
}

void checkExpansion(const Type& tt, const std::string& value, const std::string& expected) {
    auto expanded = tt.tidy(value);
    EXPECT_EQUAL(expected, expanded);
}

CASE("Test TypeTime expansions") {

    TypeTime ttime("time", Value());
    Type& tt(ttime);

    // 1 and 2-digit times
    checkExpansion(tt, "0", "0000");
    checkExpansion(tt, "00", "0000");
    checkExpansion(tt, "12", "1200");
    checkExpansion(tt, "6", "0600");
    checkExpansion(tt, "06", "0600");

    // 3 and 4-digit times
    checkExpansion(tt, "000", "0000");
    checkExpansion(tt, "0000", "0000");
    checkExpansion(tt, "012", "0012");
    checkExpansion(tt, "0012", "0012");
    checkExpansion(tt, "1234", "1234");
    checkExpansion(tt, "623", "0623");
    checkExpansion(tt, "0623", "0623");
    { EXPECT_THROWS_AS(auto vv = tt.tidy("675"), BadTime); }

    // 5 and 6-digit times
    checkExpansion(tt, "00000", "0000");
    checkExpansion(tt, "001200", "0012");
    checkExpansion(tt, "123400", "1234");
    checkExpansion(tt, "062300", "0623");
    checkExpansion(tt, "62300", "0623");
    {
        // We don't support seconds yet.
        EXPECT_THROWS_AS(auto vv = tt.tidy("000012"), BadValue);
    }
    {
        // We don't support seconds yet.
        EXPECT_THROWS_AS(auto vv = tt.tidy("123456"), BadValue);
    }
    {
        // We don't support time > 24h
        EXPECT_THROWS_AS(auto vv = tt.tidy("283456"), BadValue);
    }

    // times with colons
    checkExpansion(tt, "0:0", "0000");
    checkExpansion(tt, "00:00", "0000");
    checkExpansion(tt, "00:00:00", "0000");
    checkExpansion(tt, "0:12", "0012");
    checkExpansion(tt, "00:12", "0012");
    checkExpansion(tt, "12:34", "1234");
    checkExpansion(tt, "6:23", "0623");
    checkExpansion(tt, "06:23", "0623");
    checkExpansion(tt, "00:12:00", "0012");
    checkExpansion(tt, "12:34:00", "1234");
    checkExpansion(tt, "06:23:00", "0623");
    checkExpansion(tt, "6:23:00", "0623");
    { EXPECT_THROWS_AS(auto vv = tt.tidy("00:62"), BadTime); }
    {
        // We don't support seconds yet.
        EXPECT_THROWS_AS(auto vv = tt.tidy("00:00:12"), BadValue);
    }
    {
        // We don't support seconds yet.
        EXPECT_THROWS_AS(auto vv = tt.tidy("12:34:56"), BadValue);
    }

    // times with units
    assertTypeExpansion("time", {"0h"}, {"0000"});
    assertTypeExpansion("time", {"00H"}, {"0000"});
    assertTypeExpansion("time", {"60m"}, {"0100"});
    assertTypeExpansion("time", {"2h30m"}, {"0230"});
    assertTypeExpansion("time", {"60s"}, {"0001"});
    EXPECT_THROWS_AS(assertTypeExpansion("time", {"6s"}, {"0000"}), BadValue);
    EXPECT_THROWS_AS(assertTypeExpansion("time", {"25"}, {"0000"}), BadValue);

    // from to by
    assertTypeExpansion("time", {"0", "to", "18"}, {"0000", "0600", "1200", "1800"});
    assertTypeExpansion("time", {"0", "to", "6", "by", "1"}, {"0000", "0100", "0200", "0300", "0400", "0500", "0600"});
    assertTypeExpansion("time", {"0", "to", "1h30m", "by", "30m"}, {"0000", "0030", "0100", "0130"});
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
