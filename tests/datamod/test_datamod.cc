/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <cstdint>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "metkit/datamod/Dict.h"
#include "metkit/datamod/Json.h"

using namespace eckit::testing;
using namespace metkit::datamod;

namespace metkit::test {

CASE("set, find, replace and erase, including growth of the slot table") {
    Dict dict;
    EXPECT(dict.empty());
    EXPECT(dict.find("missing") == nullptr);

    for (int i = 0; i < 100; ++i) {
        dict.set("key" + std::to_string(i), Value{i});
    }
    EXPECT_EQUAL(dict.size(), 100);
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQUAL(dict.at("key" + std::to_string(i)).asInt(), i);
    }

    dict.set("key7", Value{"seven"});
    EXPECT_EQUAL(dict.size(), 100);
    EXPECT_EQUAL(dict.at("key7").asString(), std::string("seven"));

    EXPECT(dict.erase("key7"));
    EXPECT(!dict.erase("key7"));
    EXPECT(!dict.has("key7"));
    EXPECT_EQUAL(dict.size(), 99);
    EXPECT_EQUAL(dict.at("key99").asInt(), 99);  // indices rebuilt after erase
    EXPECT_THROWS_AS(dict.at("key7"), eckit::UserError);
}

CASE("insertion order is kept and copies are independent") {
    Dict dict;
    dict.set("b", Value{1});
    dict.set("a", Value{2});
    dict.set("c", Value{3});
    EXPECT_EQUAL(dict.entries()[0].key, std::string("b"));
    EXPECT_EQUAL(dict.entries()[2].key, std::string("c"));

    Dict copy = dict;
    copy.set("a", Value{20});
    EXPECT_EQUAL(dict.at("a").asInt(), 2);
    EXPECT(copy != dict);
}

CASE("null is a value of its own") {
    Dict dict;
    dict.set("levelist", Value{Null{}});
    EXPECT(dict.has("levelist"));
    EXPECT(dict.at("levelist").isNull());
    EXPECT_THROWS_AS(dict.at("levelist").asInt(), eckit::BadValue);
}

CASE("strict typed accessors") {
    const Value i{130};
    EXPECT(i.isInt());
    EXPECT_THROWS_AS(i.asDouble(), eckit::BadValue);
    EXPECT_THROWS_AS(i.asString(), eckit::BadValue);
    EXPECT(Value{true}.isBool());
    EXPECT(Value{0.5}.isDouble());
    EXPECT((Value{std::vector<long>{1, 2}}.isIntList()));
}

CASE("JSON round trip keeps types, order, nulls and nested objects") {
    const std::string json = R"({"mars":{"class":"od","param":130,"levelist":null,"grid":"O1280"},)"
                             R"("misc":{"pv":[1,2.5,3],"names":["a","b"],"ints":[],"flag":true},)"
                             R"("opt":{"list":[{"x":1},{"y":"z"}]}})";
    const Dict dict        = parseJsonObject(json);

    const Dict& mars = dict.at("mars").asDict();
    EXPECT_EQUAL(mars.at("class").asString(), std::string("od"));
    EXPECT_EQUAL(mars.at("param").asInt(), 130);
    EXPECT(mars.at("levelist").isNull());

    const Dict& misc = dict.at("misc").asDict();
    EXPECT(misc.at("pv").isDoubleList());
    EXPECT_EQUAL(misc.at("pv").asDoubleList()[1], 2.5);
    EXPECT(misc.at("names").isStringList());
    EXPECT(misc.at("ints").isIntList());
    EXPECT_EQUAL(misc.at("ints").listSize(), 0);
    EXPECT(misc.at("flag").asBool());

    EXPECT_EQUAL(dict.at("opt").asDict().at("list").asDictList().size(), 2);

    const Dict again = parseJsonObject(toJson(dict));
    EXPECT(again == dict);
    EXPECT_EQUAL(toJson(again), toJson(dict));
}

CASE("doubles are written exactly and stay doubles") {
    Dict dict;
    dict.set("a", Value{0.1});
    dict.set("b", Value{1.0});
    dict.set("c", Value{-1e300});
    const Dict again = parseJsonObject(toJson(dict));
    EXPECT_EQUAL(again.at("a").asDouble(), 0.1);
    EXPECT(again.at("b").isDouble());
    EXPECT_EQUAL(again.at("c").asDouble(), -1e300);
}

CASE("strings with escapes and unicode") {
    const Dict dict = parseJsonObject(R"({"s":"a\"b\\c\né😀"})");
    EXPECT_EQUAL(dict.at("s").asString(), std::string("a\"b\\c\n\xC3\xA9\xF0\x9F\x98\x80"));
    EXPECT(parseJsonObject(toJson(dict)) == dict);
}

CASE("invalid or unsupported JSON is rejected") {
    EXPECT_THROWS_AS(parseJsonObject("[1,2]"), eckit::BadValue);
    EXPECT_THROWS_AS(parseJsonObject(R"({"a":1)"), eckit::BadValue);
    EXPECT_THROWS_AS(parseJsonObject(R"({"a":[true,false]})"), eckit::BadValue);
    EXPECT_THROWS_AS(parseJsonObject(R"({"a":[1,"x"]})"), eckit::BadValue);
    EXPECT_THROWS_AS(parseJsonObject(R"({"a":1} x)"), eckit::BadValue);
}

}  // namespace metkit::test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
