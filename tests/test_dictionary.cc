/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_dictionary.cc
/// @date   Oct 2026

#include <set>
#include <string>
#include <thread>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "metkit/mars/Dictionary.h"

using namespace eckit::testing;

namespace metkit::mars::test {

//-----------------------------------------------------------------------------

CASE("test_dictionary_add_and_lookup") {
    Dictionary<Keyword> dict("keyword");

    Keyword a = dict.add("class");
    Keyword b = dict.add("type");
    EXPECT(a != 0);
    EXPECT(b != 0);
    EXPECT(a != b);

    // adding again returns the same key
    EXPECT_EQUAL(dict.add("class"), a);

    EXPECT_EQUAL(dict.exist("class"), a);
    EXPECT_EQUAL(dict.exist("unknown"), 0);
    EXPECT_EQUAL(dict.keyword("type"), b);
    EXPECT_EQUAL(dict.name(a), "class");

    // an unknown name is an error of the user, an invalid key a bug
    EXPECT_THROWS_AS(dict.keyword("unknown"), eckit::UserError);
    EXPECT_THROWS_AS(dict.name(0), eckit::SeriousBug);
    EXPECT_THROWS_AS(dict.name(1000), eckit::SeriousBug);
}

CASE("test_dictionary_alias") {
    Dictionary<Keyword> dict("keyword");
    Keyword a = dict.add("levelist");
    Keyword b = dict.add("param");

    dict.alias("level", a);
    EXPECT_EQUAL(dict.exist("level"), a);
    EXPECT_EQUAL(dict.name(a), "levelist");

    // the same alias for the same key is fine, for another key it is a conflict
    EXPECT_NO_THROW(dict.alias("level", a));
    EXPECT_THROWS_AS(dict.alias("level", b), eckit::SeriousBug);

    // an alias must refer to an existing key
    EXPECT_THROWS_AS(dict.alias("other", 0), eckit::SeriousBug);
    EXPECT_THROWS_AS(dict.alias("other", 1000), eckit::SeriousBug);
}

CASE("test_dictionary_full") {
    // the keys must stay representable: an overflow must not wrap around (and clash with existing keys)
    Dictionary<uint8_t> dict("verb");

    size_t added = 0;
    EXPECT_THROWS_AS(
        [&] {
            for (int i = 0; i < 1000; ++i) {
                uint8_t key = dict.add("verb" + std::to_string(i));
                EXPECT(key != 0);
                ++added;
            }
        }(),
        eckit::SeriousBug);
    EXPECT_EQUAL(added, 254u);

    // the existing entries are untouched
    EXPECT_EQUAL(dict.exist("verb0"), 1);
    EXPECT_EQUAL(dict.name(254), "verb253");
}

CASE("test_dictionary_concurrent_add") {
    Dictionary<Keyword> dict("keyword");

    constexpr int numThreads = 8;
    constexpr int numNames   = 500;
    std::vector<std::vector<Keyword>> keys(numThreads, std::vector<Keyword>(numNames));

    std::vector<std::thread> threads;
    for (int t = 0; t < numThreads; ++t) {
        threads.emplace_back([&, t] {
            for (int i = 0; i < numNames; ++i) {
                keys[t][i] = dict.add("name" + std::to_string(i));
            }
        });
    }
    for (auto& th : threads) {
        th.join();
    }

    // every thread got the same key for the same name, and the keys are all different
    std::set<Keyword> distinct;
    for (int i = 0; i < numNames; ++i) {
        for (int t = 1; t < numThreads; ++t) {
            EXPECT_EQUAL(keys[t][i], keys[0][i]);
        }
        distinct.insert(keys[0][i]);
        EXPECT_EQUAL(dict.name(keys[0][i]), "name" + std::to_string(i));
    }
    EXPECT_EQUAL(distinct.size(), static_cast<size_t>(numNames));
}

//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
