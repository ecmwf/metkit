/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_levelist.cc
/// @author Metin Cakircali
/// @author Emanuele Danovaro
/// @date   November 2023

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/TypeFloat.h"
#include "metkit/mars/TypesFactory.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

CASE("test_metkit_exists_to-by-list-float") {
    std::stringstream ss;
    TypesFactory::list(ss);
    const std::string str(ss.str());
    EXPECT(str.find_first_of("to-by-list-float"));
}

void assertTypeExpansion(const std::string& name, std::vector<std::string> values,
                         const std::vector<std::string>& expected) {
    static MarsLanguage language("retrieve");
    language.type(name)->expand(DummyContext(), values);
    ASSERT(values == expected);
}

CASE("test_metkit_expand_levelist") {
    // by > 0
    assertTypeExpansion("levelist", {"-1", "to", "2", "by", "0.5"}, {"-1", "-0.5", "0", ".5", "1", "1.5", "2"});
    // by > 0
    assertTypeExpansion("levelist", {"-10.0", "to", "2.0", "by", "1"},
                        {"-10", "-9", "-8", "-7", "-6", "-5", "-4", "-3", "-2", "-1", "0", "1", "2"});
    //  by > 0 && from < to
    assertTypeExpansion("levelist", {"4", "to", "20", "by", "4"}, {"4", "8", "12", "16", "20"});
    assertTypeExpansion("levelist", {"4", "to", "18", "by", "4"}, {"4", "8", "12", "16"});
    // by > 0 && from > to
    assertTypeExpansion("levelist", {"20", "to", "4", "by", "4"}, {"20", "16", "12", "8", "4"});
    // by = 0
    EXPECT_THROWS_AS(assertTypeExpansion("levelist", {"4", "to", "20", "by", "0"}, {"4"}), eckit::BadValue);
    EXPECT_THROWS_AS(assertTypeExpansion("levelist", {"-1", "to", "2", "by", "0"}, {"-1"}), eckit::BadValue);
    // by < 0 && from > to
    assertTypeExpansion("levelist", {"20", "to", "4", "by", "-4"}, {"20", "16", "12", "8", "4"});
    assertTypeExpansion("levelist", {"10", "to", "4", "by", "-2"}, {"10", "8", "6", "4"});
    assertTypeExpansion("levelist", {"-2", "to", "-4", "by", "-0.5"}, {"-2", "-2.5", "-3", "-3.5", "-4"});
    assertTypeExpansion("levelist", {"0", "to", "-2", "by", "-0.5"}, {"0", "-0.5", "-1", "-1.5", "-2"});
    // by < 0 && from < to
    EXPECT_THROWS_AS(assertTypeExpansion("levelist", {"4", "to", "10", "by", "-4"}, {"4", "8", "12", "16", "20"}), eckit::BadValue);
    EXPECT_THROWS_AS(assertTypeExpansion("levelist", {"-4", "to", "2", "by", "-0.5"}, {"0", "-0.5", "-1", "-1.5", "-2"}), eckit::BadValue);
}

}  // namespace test
}  // namespace mars
}  // namespace metkit

//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
