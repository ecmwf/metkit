/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <algorithm>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"
#include "eckit/types/Date.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypeAny.h"
#include "metkit/mars/TypeDate.h"
#include "metkit/mars/TypeEnum.h"
#include "metkit/mars/TypeExpver.h"
#include "metkit/mars/TypeMixed.h"
#include "metkit/mars/TypeParam.h"
#include "metkit/mars/TypesFactory.h"

#define EXPECT_EMPTY(request, key) EXPECT_EQUAL(request.values(key, true).size(), 0);

namespace metkit::mars::test {

struct Expected {
    const std::string verb;
    StringManyMap keyvalue;
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

CASE("check language file") {

    EXPECT_THROWS(MarsLanguage::jsonFile("langauge.yaml"));

    eckit::Value file;

    EXPECT_NO_THROW(file = MarsLanguage::jsonFile("language.yaml"));

    EXPECT(file.contains("disseminate"));
    EXPECT(file.contains("archive"));
    EXPECT(file.contains("retrieve"));
    EXPECT(file.contains("read"));
    EXPECT(file.contains("get"));
    EXPECT(file.contains("list"));
    EXPECT(file.contains("compute"));
    EXPECT(file.contains("write"));
    EXPECT(file.contains("pointdb"));
}

CASE("check defaults and _clear_defaults") {

    {
        // check "retrieve" that _clear_defaults is not set
        auto request = MarsRequest::parse("retrieve,param=2t,step=10/to/12/by/1", true);
        EXPECT_EQUAL(request.values("class").size(), 1);
        EXPECT_EQUAL(request.values("class")[0], "od");
        // check date is yesterday
        auto values = request.values("date");
        EXPECT_EQUAL(values.size(), 1);
        std::ostringstream yesterday;
        yesterday << eckit::Date(-1).yyyymmdd();
        EXPECT_EQUAL(values[0], yesterday.str());
    }

    {
        // check "read" that _clear_defaults is set for class
        auto request = MarsRequest::parse("read,param=2t", true);
        EXPECT_EMPTY(request, "class");
        EXPECT_EMPTY(request, "date");
        EXPECT_EMPTY(request, "domain");
        EXPECT_EMPTY(request, "expver");
        EXPECT_EMPTY(request, "levelist");
        EXPECT_EMPTY(request, "levtype");
        EXPECT_EMPTY(request, "step");
        EXPECT_EMPTY(request, "stream");
        EXPECT_EMPTY(request, "time");
        EXPECT_EMPTY(request, "type");
    }

    {
        // check "list" that _clear_defaults is set for class
        auto request = MarsRequest::parse("list,param=2t", true);
        EXPECT_EQUAL(request.values("class").size(), 1);
        EXPECT_EQUAL(request.values("class")[0], "od");
        EXPECT_EMPTY(request, "date");
        EXPECT_EMPTY(request, "domain");
        EXPECT_EMPTY(request, "expver");
        EXPECT_EMPTY(request, "levelist");
        EXPECT_EMPTY(request, "levtype");
        EXPECT_EMPTY(request, "step");
        EXPECT_EMPTY(request, "stream");
        EXPECT_EMPTY(request, "time");
        EXPECT_EMPTY(request, "type");
    }

    {
        // check "pointdb" that _clear_defaults is set for class
        auto request = MarsRequest::parse("pointdb,param=2t", true);
        EXPECT_EMPTY(request, "class");
        EXPECT_EMPTY(request, "date");
        EXPECT_EMPTY(request, "domain");
        EXPECT_EMPTY(request, "expver");
        EXPECT_EMPTY(request, "levelist");
        EXPECT_EMPTY(request, "levtype");
        EXPECT_EMPTY(request, "step");
        EXPECT_EMPTY(request, "stream");
        EXPECT_EMPTY(request, "time");
        EXPECT_EMPTY(request, "type");
    }
}

CASE("check method: isData()") {

    EXPECT_EQUAL(MarsLanguage("retrieve").isData("class"), true);
    EXPECT_EQUAL(MarsLanguage("retrieve").isData("date"), true);
    EXPECT_EQUAL(MarsLanguage("retrieve").isData("time"), true);
    EXPECT_EQUAL(MarsLanguage("retrieve").isData("step"), true);
    EXPECT_EQUAL(MarsLanguage("retrieve").isData("number"), true);

    EXPECT_EQUAL(MarsLanguage("disseminate").isData("accuracy"), false);
    EXPECT_EQUAL(MarsLanguage("disseminate").isData("grid"), false);
}

CASE("check method: flatten()") {

    struct Output : public FlattenCallback {
        std::ostringstream oss;
        void operator()(const MarsRequest& request) override { oss << request << '\n'; }
    };

    Output output;

    auto request = MarsRequest::parse(
        "retrieve,class=od,type=an,stream=oper,levtype=pl,time=1200,param=2t,step=10/to/14/by/2,levelist=300/400/"
        "500,date=20250717",
        true);

    MarsLanguage("retrieve").flatten(DummyContext(), request, output);

    EXPECT_EQUAL(output.oss.str(),
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=10,levelist=300,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=10,levelist=400,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=10,levelist=500,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=12,levelist=300,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=12,levelist=400,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=12,levelist=500,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=14,levelist=300,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=14,levelist=400,param="
                 "167,expver=0001,domain=g\n"
                 "retrieve,class=od,type=an,stream=oper,levtype=pl,date=20250717,time=1200,step=14,levelist=500,param="
                 "167,expver=0001,domain=g\n");
}

CASE("check some types") {

    {
        EXPECT_THROWS(MarsLanguage("read").type("unknown"));

        EXPECT_THROWS(MarsLanguage("retrieve").type("unknown"));

        EXPECT_NO_THROW(MarsLanguage("retrieve").type("_hidden"));
    }

    {
        auto language = MarsLanguage("retrieve");

        auto* type = language.type("class");
        EXPECT(dynamic_cast<TypeEnum*>(type) != nullptr);

        type = language.type("param");
        EXPECT(dynamic_cast<TypeParam*>(type) != nullptr);

        type = language.type("expver");
        EXPECT(dynamic_cast<TypeExpver*>(type) != nullptr);

        type = language.type("domain");
        EXPECT(dynamic_cast<TypeMixed*>(type) != nullptr);

        type = language.type("date");
        EXPECT(dynamic_cast<TypeDate*>(type) != nullptr);

        type = language.type("grid");
        EXPECT(dynamic_cast<TypeMixed*>(type) != nullptr);
        EXPECT_EQUAL(type->multiple(), true);

        type = language.type("area");
        EXPECT(dynamic_cast<TypeMixed*>(type) != nullptr);
        EXPECT_EQUAL(type->multiple(), true);

        type = language.type("accuracy");
        EXPECT(dynamic_cast<TypeMixed*>(type) != nullptr);

        type = language.type("resol");
        EXPECT(dynamic_cast<TypeMixed*>(type) != nullptr);
    }
    {
        auto language = MarsLanguage("archive");

        auto* type = language.type("resol");
        EXPECT(dynamic_cast<TypeAny*>(type) != nullptr);

        EXPECT_THROWS_AS(language.type("grid"), eckit::SeriousBug);
        EXPECT_THROWS_AS(language.type("area"), eckit::SeriousBug);
    }
}

// -----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
