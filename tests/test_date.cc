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
#include "eckit/types/Date.h"
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
    language.type(name)->expand(DummyContext{}, values);
    EXPECT_EQUAL(expected, values);
}

std::string date(long d) {
    if (d <= 0) {
        eckit::Date day(d);
        d = day.yyyymmdd();
    }
    return std::to_string(d);
}

CASE("Test TypeDate expansions") {

    TypeDate tdate("date", Value());
    Type& td(tdate);

    assertTypeExpansion("date", {"20140506"}, {"20140506"});
    assertTypeExpansion("date", {"2014-05-06"}, {"20140506"});
    assertTypeExpansion("date", {"20140506", "20140507"}, {"20140506", "20140507"});
    assertTypeExpansion("date", {"20140506", "to", "20140506"}, {"20140506"});
    assertTypeExpansion("date", {"20140506", "to", "20140507"}, {"20140506", "20140507"});
    assertTypeExpansion("date", {"20140506", "to", "20140508"}, {"20140506", "20140507", "20140508"});
    assertTypeExpansion("date", {"20140504", "20140506", "to", "20140508"},
                        {"20140504", "20140506", "20140507", "20140508"});

    assertTypeExpansion("date", {"-1", "0"}, {date(-1), date(0)});
    assertTypeExpansion("date", {"-1", "to", "-3"}, {date(-1), date(-2), date(-3)});
    assertTypeExpansion("date", {"-3", "to", "-1"}, {date(-3), date(-2), date(-1)});
    assertTypeExpansion("date", {"-5", "to", "-1", "by", "2"}, {date(-5), date(-3), date(-1)});

    assertTypeExpansion("date", {"2"}, {"feb"});
    assertTypeExpansion("date", {"jan"}, {"jan"});
    assertTypeExpansion("date", {"september"}, {"sep"});
    assertTypeExpansion("date", {"9"}, {"sep"});
    assertTypeExpansion("date", {"1-01"}, {"jan-1"});
    assertTypeExpansion("date", {"jan-01"}, {"jan-1"});
    assertTypeExpansion("date", {"january-01"}, {"jan-1"});
    assertTypeExpansion("date", {"feb-23"}, {"feb-23"});
    assertTypeExpansion("date", {"2018-23"}, {"20180123"});
    assertTypeExpansion("date", {"2018-41"}, {"20180210"});

    {
        EXPECT_THROWS(auto vv = td.tidy("20141506"));  // throws BadDate that is not exported
    }
    {
        EXPECT_THROWS(auto vv = td.tidy("20180132"));  // throws BadDate that is not exported
    }
    {
        EXPECT_THROWS(auto vv = td.tidy("202401366"));  // throws BadDate that is not exported
    }
    { EXPECT_THROWS_AS(auto vv = td.tidy("abc"), BadValue); }
    { EXPECT_THROWS_AS(auto vv = td.tidy("abc-01"), BadValue); }
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
