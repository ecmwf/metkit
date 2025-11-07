/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_step.cc
/// @author Emanuele Danovaro
/// @date   March 2025

#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"

namespace metkit::mars::test {

using ::eckit::BadValue;

//-----------------------------------------------------------------------------

void assertTypeExpansion(const std::string& name, std::vector<std::string> values,
                         const std::vector<std::string>& expected) {
    static MarsLanguage language("retrieve");
    language.type(name)->expand(values);
    EXPECT_EQUAL(expected, values);
}

CASE("Test Step expansions") {

    // times with units
    assertTypeExpansion("step", {"0"}, {"0"});
    assertTypeExpansion("step", {"12"}, {"12"});
    assertTypeExpansion("step", {"260m"}, {"4h20m"});
    assertTypeExpansion("step", {"30m", "1h", "1h30m", "120m"}, {"30m", "1", "1h30m", "2"});
    assertTypeExpansion("step", {"0-1"}, {"0-1"});
    assertTypeExpansion("step", {"30m-60m"}, {"30m-1"});
    EXPECT_THROWS_AS(assertTypeExpansion("step", {"2-1"}, {""}), BadValue);

    assertTypeExpansion("step", {"0-3", "to", "9-12", "by", "3h"}, {"0-3", "3-6", "6-9", "9-12"});
    assertTypeExpansion("step", {"0-3", "to", "0-12", "by", "3"}, {"0-3"});
    assertTypeExpansion("step", {"0-30m", "to", "1h30m-2h", "by", "30m"}, {"0-30m", "30m-1", "1-1h30m", "1h30m-2"});
    assertTypeExpansion(
        "step", {"0m", "to", "1440m", "by", "10m"},
        {"0",  "10m",    "20m",    "30m",    "40m",    "50m",    "1",  "1h10m",  "1h20m",  "1h30m",  "1h40m",  "1h50m",
         "2",  "2h10m",  "2h20m",  "2h30m",  "2h40m",  "2h50m",  "3",  "3h10m",  "3h20m",  "3h30m",  "3h40m",  "3h50m",
         "4",  "4h10m",  "4h20m",  "4h30m",  "4h40m",  "4h50m",  "5",  "5h10m",  "5h20m",  "5h30m",  "5h40m",  "5h50m",
         "6",  "6h10m",  "6h20m",  "6h30m",  "6h40m",  "6h50m",  "7",  "7h10m",  "7h20m",  "7h30m",  "7h40m",  "7h50m",
         "8",  "8h10m",  "8h20m",  "8h30m",  "8h40m",  "8h50m",  "9",  "9h10m",  "9h20m",  "9h30m",  "9h40m",  "9h50m",
         "10", "10h10m", "10h20m", "10h30m", "10h40m", "10h50m", "11", "11h10m", "11h20m", "11h30m", "11h40m", "11h50m",
         "12", "12h10m", "12h20m", "12h30m", "12h40m", "12h50m", "13", "13h10m", "13h20m", "13h30m", "13h40m", "13h50m",
         "14", "14h10m", "14h20m", "14h30m", "14h40m", "14h50m", "15", "15h10m", "15h20m", "15h30m", "15h40m", "15h50m",
         "16", "16h10m", "16h20m", "16h30m", "16h40m", "16h50m", "17", "17h10m", "17h20m", "17h30m", "17h40m", "17h50m",
         "18", "18h10m", "18h20m", "18h30m", "18h40m", "18h50m", "19", "19h10m", "19h20m", "19h30m", "19h40m", "19h50m",
         "20", "20h10m", "20h20m", "20h30m", "20h40m", "20h50m", "21", "21h10m", "21h20m", "21h30m", "21h40m", "21h50m",
         "22", "22h10m", "22h20m", "22h30m", "22h40m", "22h50m", "23", "23h10m", "23h20m", "23h30m", "23h40m", "23h50m",
         "24"});
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
