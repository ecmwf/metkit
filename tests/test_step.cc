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

#include "eckit/testing/Test.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/TypeTime.h"

using namespace eckit;
using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

void assertTypeExpansion(const std::string& name, std::vector<std::string> values,
                         const std::vector<std::string>& expected) {
    static MarsLanguage language("retrieve");
    language.type(name)->expand(DummyContext(), values);
    EXPECT_EQUAL(expected, values);
}

CASE("Test TypeRange expansions") {

    // times with units
    assertTypeExpansion("step", {"0"}, {"0"});
    assertTypeExpansion("step", {"12"}, {"12"});
    assertTypeExpansion("step", {"30m", "1h", "1h30m", "120m"}, {"30m", "1", "1h30m", "2"});
    assertTypeExpansion("step", {"0-1"}, {"0-1"});
    assertTypeExpansion("step", {"30m-60m"}, {"30m-1"});
    EXPECT_THROWS_AS(assertTypeExpansion("step", {"2-1"}, {""}), BadValue);

    assertTypeExpansion("step", {"0-3", "to", "9-12", "by", "3h"}, {"0-3", "3-6", "6-9", "9-12"});
    assertTypeExpansion("step", {"0-3", "to", "0-12", "by", "3"}, {"0-3"});
    assertTypeExpansion("step", {"0-30m", "to", "1h30m-2h", "by", "30m"}, {"0-30m", "30m-1", "1-1h30m", "1h30m-2"});
}


//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
