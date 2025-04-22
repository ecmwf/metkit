/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <map>
#include <random>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"


namespace metkit::mars::test {

struct Expected {
    const std::string verb;
    std::map<std::string, std::vector<std::string>> keyvalue;
};

using Sequence = std::vector<unsigned long>;

Sequence make_random_sequence(Sequence::value_type min, Sequence::value_type max, Sequence::size_type N) {
    static std::mt19937 gen{std::random_device{}()};
    static std::uniform_int_distribution<Sequence::value_type> dist(min, max);

    Sequence s(N);
    std::generate(s.begin(), s.end(), [&] { return dist(gen); });
    return s;
}

Sequence make_power_sequence(Sequence::value_type min, Sequence::value_type max) {
    Sequence s;
    for (auto i = min; i <= max; i *= 2) {
        s.push_back(i);
    }
    return s;
}

void expect_mars(const std::string& text, const Expected& expected, bool strict = false) {
    auto r = MarsRequest::parse(text, strict);
    EXPECT_EQUAL(expected.verb, r.verb());

    for (const auto& kv : expected.keyvalue) {
        auto v = r.values(kv.first);

        if (kv.first == "grid") {
            EXPECT(kv.second.size() == 1 && v.size() == 1 && v[0] == kv.second[0]);
            continue;
        }

        EXPECT_EQUAL(kv.second, kv.second);
    }
}

// -----------------------------------------------------------------------------

CASE("grid: regular Gaussian grids") {
    const Sequence F{16,  24,  32,  48,  64,  80,  96,   128,  160,  192,  200,  256,  320,
                     400, 512, 576, 640, 800, 912, 1024, 1280, 1600, 2000, 2560, 4000, 8000};

    const auto Fn = [](const auto& other) {
        auto seq = make_random_sequence(2, 8000, 20);
        seq.insert(seq.end(), other.begin(), other.end());
        return seq;
    }(F);

    for (const auto& n : Fn) {
        // known grids are expanded correctly (pattern matching doesn't work reliably here)
        Expected expected{"retrieve", {{"grid", {"F" + std::to_string(n)}}}};
        if (std::find(F.begin(), F.end(), n) == F.end()) {
            expected.keyvalue.clear();
        }

        expect_mars("ret, date=-1, grid=F" + std::to_string(n), expected);
        expect_mars("ret, date=-1, grid=f" + std::to_string(n), expected);
        expect_mars("ret, date=-1, grid=" + std::to_string(n), expected);
    }
}

// -----------------------------------------------------------------------------

CASE("grid: octahedral Gaussian grids") {
    for (const auto& n : make_random_sequence(2, 8000, 20 /*reduced number of tests*/)) {
        const Expected expected{"retrieve", {{"grid", {"O" + std::to_string(n)}}}};
        expect_mars("ret, date=-1, grid=O" + std::to_string(n), expected);
        expect_mars("ret, date=-1, grid=o" + std::to_string(n), expected);
    }
}

// -----------------------------------------------------------------------------

CASE("grid: reduced classical Gaussian grids") {
    for (const auto& n : Sequence{32, 48, 64, 80, 96, 128, 160, 200, 256, 320, 400, 512, 640, 800, 1024, 1280, 8000}) {
        const Expected expected{"retrieve", {{"grid", {"N" + std::to_string(n)}}}};
        expect_mars("ret, date=-1, grid=N" + std::to_string(n), expected);
        expect_mars("ret, date=-1, grid=n" + std::to_string(n), expected);
    }
}

// -----------------------------------------------------------------------------

CASE("grid: HEALPix grids") {
    for (const auto& n : make_power_sequence(2, 8192)) {
        const Expected expected{"retrieve", {{"grid", {"H" + std::to_string(n)}}}};
        expect_mars("ret, date=-1, grid=H" + std::to_string(n), expected);
        expect_mars("ret, date=-1, grid=h" + std::to_string(n), expected);
    }
}

// -----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
