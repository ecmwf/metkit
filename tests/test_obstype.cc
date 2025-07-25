/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_obstype.cc
/// @author Emanuele Danovaro
/// @date   July 2025

#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"

namespace metkit::mars::test {

using ::eckit::UserError;

//-----------------------------------------------------------------------------

void assertTypeExpansion(const std::string& name, std::vector<std::string> values,
                         const std::vector<std::string>& expected) {
    static MarsLanguage language("retrieve");
    language.type(name)->expand(DummyContext{}, values);
    EXPECT_EQUAL(expected, values);
}

CASE("Test Obstype expansions") {

    // times with units
    assertTypeExpansion("obstype", {"1"}, {"1"});
    assertTypeExpansion("obstype", {"ssmi"}, {"126"});
    assertTypeExpansion("obstype", {"trmm"}, {"129", "130"});
    assertTypeExpansion("obstype", {"ti3r", "trmm"}, {"130", "129"});
    assertTypeExpansion("obstype", {"130", "trmm"}, {"130", "129"});
    assertTypeExpansion("obstype", {"trmm", "qscat"}, {"129", "130", "137", "138"});
    assertTypeExpansion("obstype", {"sd"},
                        {"121", "122", "123", "124", "210", "212", "213", "214", "216", "217", "218", "51",  "53",
                         "54",  "55",  "56",  "57",  "59",  "60",  "61",  "62",  "63",  "65",  "71",  "72",  "73",
                         "75",  "138", "139", "153", "155", "211", "240", "250", "126", "49",  "127", "129", "130",
                         "137", "206", "207", "208", "209", "156", "154", "201", "202", "252", "245", "246"});
    EXPECT_THROWS_AS(assertTypeExpansion("obstype", {"foo"}, {""}), UserError);
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
