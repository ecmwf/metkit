// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   test_dictionary.cc
/// @date   Oct 2026

#include <set>
#include <string>
#include <thread>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "metkit/mars/MarsKeyword.h"
#include "metkit/mars/MarsVerb.h"
#include "metkit/mars/SymbolTable.h"

using namespace eckit::testing;

namespace metkit::mars::test {

//-----------------------------------------------------------------------------

CASE("test_dictionary_add_and_lookup") {
    SymbolTable<MarsKeyword> dict("keyword");

    MarsKeyword a = dict.add("class");
    MarsKeyword b = dict.add("type");
    EXPECT(a != MarsKeyword::unknown());
    EXPECT(b != MarsKeyword::unknown());
    EXPECT(a != b);

    // adding again returns the same key
    EXPECT_EQUAL(dict.add("class"), a);

    EXPECT_EQUAL(dict.symbol("class", false), a);
    EXPECT_EQUAL(dict.symbol("unknown", false), MarsKeyword::unknown());
    EXPECT_EQUAL(dict.symbol("type"), b);
    EXPECT_EQUAL(dict.name(a), "class");

    // an unknown name is an error of the user, an invalid key a bug
    EXPECT_THROWS_AS(dict.symbol("unknown"), eckit::UserError);
    EXPECT_THROWS_AS(dict.name(MarsKeyword::unknown()), eckit::SeriousBug);
}

CASE("test_dictionary_alias") {
    SymbolTable<MarsKeyword> dict("keyword");
    MarsKeyword a = dict.add("levelist");
    MarsKeyword b = dict.add("param");

    dict.addAlias("level", a);
    EXPECT_EQUAL(dict.symbol("level", false), a);
    EXPECT_EQUAL(dict.name(a), "levelist");

    // the same alias for the same key is fine, for another key it is a conflict
    EXPECT_NO_THROW(dict.addAlias("level", a));
    EXPECT_THROWS_AS(dict.addAlias("level", b), eckit::SeriousBug);

    // an alias must refer to an existing key
    EXPECT_THROWS_AS(dict.addAlias("other", 0), eckit::SeriousBug);
    EXPECT_THROWS_AS(dict.addAlias("other", 1000), eckit::SeriousBug);
}

CASE("test_dictionary_full") {
    // the keys must stay representable: an overflow must not wrap around (and clash with existing keys)
    SymbolTable<MarsVerb> dict("verb");

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
    EXPECT_EQUAL(dict.symbol("verb0", false), MarsVerb{1});
    EXPECT_EQUAL(dict.name(254), "verb253");
}

CASE("test_dictionary_concurrent_add") {
    SymbolTable<MarsKeyword> dict("keyword");

    constexpr int numThreads = 8;
    constexpr int numNames   = 500;
    std::vector<std::vector<MarsKeyword>> keys(numThreads, std::vector<MarsKeyword>(numNames));

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
    std::set<MarsKeyword> distinct;
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
