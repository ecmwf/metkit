/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_request.cc
/// @date   Jul 2024
/// @author Emanuele Danovaro

#include <sstream>
#include <string>
#include <type_traits>

#include "eckit/io/Buffer.h"
#include "eckit/log/JSON.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/serialisation/ResizableMemoryStream.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit::mars::test {

//-----------------------------------------------------------------------------

CASE("test_request_json") {
    {
        const char* text =
            "retrieve,class=od,expver=0079,stream=enfh,date=20240729,time=00/"
            "12,type=fcmean,levtype=sfc,step=24,number=1/to/2,param=mucin/mucape/tprate";
        MarsRequest r = MarsRequest::parse(text);
        {
            std::stringstream ss;
            eckit::JSON plain(ss);
            r.json(plain);
            EXPECT_EQUAL(
                ss.str(),
                "{\"class\":\"od\",\"type\":\"fcmean\",\"stream\":\"enfh\",\"levtype\":\"sfc\",\"date\":\"20240729\","
                "\"time\":[\"0000\",\"1200\"],\"step\":\"24\",\"expver\":\"0079\",\"number\":[\"1\",\"2\"],\"param\":["
                "\"228236\",\"228235\",\"172228\"],\"domain\":\"g\"}");
        }
        {
            std::stringstream ss;
            eckit::JSON array(ss);
            r.json(array, true);
            EXPECT_EQUAL(
                ss.str(),
                "{\"class\":\"od\",\"type\":\"fcmean\",\"stream\":\"enfh\",\"levtype\":\"sfc\",\"date\":[\"20240729\"],"
                "\"time\":[\"0000\",\"1200\"],\"step\":[\"24\"],\"expver\":\"0079\",\"number\":[\"1\",\"2\"],\"param\":"
                "[\"228236\",\"228235\",\"172228\"],\"domain\":\"g\"}");
        }
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=wave,date=20240729,time=00,type=an,levtype=sfc,step=24,param=2dfd ";
        MarsRequest r = MarsRequest::parse(text);
        {
            std::stringstream ss;
            eckit::JSON plain(ss);
            r.json(plain);
            EXPECT_EQUAL(ss.str(),
                         "{\"class\":\"od\",\"type\":\"an\",\"stream\":\"wave\",\"levtype\":\"sfc\",\"date\":"
                         "\"20240729\",\"time\":\"0000\",\"step\":\"24\",\"expver\":\"0001\",\"param\":\"140251\","
                         "\"domain\":\"g\"}");
        }
        {
            std::stringstream ss;
            eckit::JSON array(ss);
            r.json(array, true);
            EXPECT_EQUAL(ss.str(),
                         "{\"class\":\"od\",\"type\":\"an\",\"stream\":\"wave\",\"levtype\":\"sfc\",\"date\":["
                         "\"20240729\"],\"time\":[\"0000\"],\"step\":[\"24\"],\"expver\":\"0001\",\"param\":["
                         "\"140251\"],\"domain\":\"g\"}");
        }
    }
}

CASE("test_request_count") {
    {
        const char* text =
            "retrieve,class=od,expver=0079,stream=enfh,date=20240729,time=00/"
            "12,type=fcmean,levtype=sfc,step=24,number=1/to/2,param=mucin/mucape/tprate";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(12, r.count());
    }
    {
        const char* text =
            "retrieve,class=od,expver=0079,stream=enfh,date=20240729,time=00/"
            "12,type=fcmean,levtype=sfc,step=24,number=1/to/2,param=mucin/mucape/tprate,area=12/13/14/15,grid=.1/.1";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(12, r.count());
    }
    {
        const char* text =
            "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/to/"
            "137,levtype=ml,number=-1,param=z,process=local,step=000,stream=scda,time=18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(1, r.count());
    }
    {
        const char* text =
            "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/to/137,levtype=ml,number=-1,param=z/"
            "t,process=local,step=000,stream=scda,time=18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(138, r.count());
    }
    {
        const char* text =
            "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/to/137,levtype=ml,number=-1,param=22/127/"
            "128/129/152/u/v,process=local,step=000,stream=scda,time=18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(279, r.count());
    }
}

CASE("test_request_count_single_level_params") {
    // Test that single-level parameters (like 'z') are only counted when level "1" is in levelist
    {
        // levelist includes "1" - single-level param 'z' should be counted
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=1/to/"
            "137,levtype=ml,param=z,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(1, r.count());  // Only level 1 for param z
    }
    {
        // levelist does NOT include "1" - single-level param 'z' should NOT be counted
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=2/to/"
            "137,levtype=ml,param=z,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(0, r.count());  // No levels counted for param z when level 1 is absent
    }
    {
        // levelist includes "1" - mix of single-level (z) and multi-level (t) params
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=1/to/137,levtype=ml,param=z/"
            "t,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(138, r.count());  // 137 levels for t + 1 for z
    }
    {
        // levelist does NOT include "1" - only multi-level param 't' should be counted
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=2/to/137,levtype=ml,param=z/"
            "t,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(136, r.count());  // 136 levels (2-137) for t, 0 for z
    }
    {
        // levelist includes "1" in the middle of range - single-level params should be counted
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=1/50/100,levtype=ml,param=z/"
            "t,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(4, r.count());  // 3 levels for t + 1 for z
    }
    {
        // levelist with only level "1" - both param types should be counted
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=1,levtype=ml,param=z/"
            "t,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(2, r.count());  // 1 level for t + 1 for z
    }
    {
        // Multiple single-level params with level "1" present
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=1/to/10,levtype=ml,param=152/"
            "z,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(2, r.count());  // 1 for z + 1 for 152 (both single-level params)
    }
    {
        // Multiple single-level params without level "1"
        const char* text =
            "retrieve,class=od,date=20230810,expver=1,levelist=2/to/10,levtype=ml,param=152/"
            "z,step=000,stream=scda,time=18,type=an";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(0, r.count());  // 0 for both single-level params when level 1 is absent
    }
    {
        const char* text =
            "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/to/137,levtype=ml,number=-1,param=22/127/"
            "128/129/152/u/v,process=local,step=000,stream=scda,time=6/18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(558, r.count());
    }
    {
        const char* text =
            "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=3/to/137,levtype=ml,number=-1,param=22/127/"
            "128/129/152/u/v,process=local,step=000,stream=scda,time=6/18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(540, r.count());
    }
    {
        const char* text =
            "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/2,levtype=ml,number=-1,param=22/127/"
            "128/129/152/u/v,process=local,step=0/1/2,stream=scda,time=6/18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(54, r.count());
    }
}

CASE("test_raw_request_stream_roundtrip") {
    // a request decoded from a stream must be fully usable: lookups go through the parameter index
    MarsRequest r("retrieve");
    r.setValue("class", "od");
    r.values("param", std::vector<std::string>{"t", "z"});

    eckit::Buffer buffer(4096);
    eckit::ResizableMemoryStream out(buffer);
    out << r;

    eckit::MemoryStream in(buffer);
    MarsRequest decoded(in);

    EXPECT_EQUAL(decoded.verb(), "retrieve");
    EXPECT(decoded.has("class"));
    EXPECT(decoded.find("param") != nullptr);
    EXPECT_EQUAL(decoded["class"], "od");
    EXPECT_EQUAL(decoded.values("param").size(), 2u);
    EXPECT_EQUAL(decoded.params().size(), 2u);
}

CASE("test_request_unknown_names") {
    MarsRequest r("retrieve");
    r.setValue("class", "od");

    // queries about a name that is not a keyword are not errors
    EXPECT(r.has("class"));
    EXPECT(!r.has("not_a_keyword"));
    EXPECT(r.find("not_a_keyword") == nullptr);
    EXPECT_EQUAL(r.countValues("not_a_keyword"), 0u);
    EXPECT(r.values("not_a_keyword", true).empty());
    EXPECT_NO_THROW(r.unsetValues("not_a_keyword"));

    // ... but asking for the values of a missing parameter is the user's error
    EXPECT_THROWS_AS(r.values("not_a_keyword"), eckit::UserError);

    // a request accepts any keyword: it is validated when it is expanded
    EXPECT_NO_THROW(r.setValue("not_a_keyword", "x"));
    EXPECT(r.has("not_a_keyword"));
    EXPECT_EQUAL(r["not_a_keyword"], "x");
    EXPECT_THROWS_AS(MarsExpansion{true}.expand(r), eckit::UserError);

    r.unsetValues("not_a_keyword");
    EXPECT(!r.has("not_a_keyword"));
    EXPECT_EQUAL(r.params().size(), 1u);
}

CASE("test_request_untyped_parameters") {
    MarsRequest r("retrieve");
    r.values("custom", std::vector<std::string>{"a", "b"});
    r.setValue("class", "od");

    // an untyped parameter keeps all its values, and accepts several
    const Parameter* custom = r.find("custom");
    EXPECT(custom != nullptr);
    EXPECT(!custom->typed());
    EXPECT(custom->multiple());
    EXPECT_EQUAL(custom->count(), 2u);
    EXPECT_THROWS_AS(custom->type(), eckit::SeriousBug);

    // the parameters of registered keywords are found by name and by keyword, and are not duplicated
    Keyword klass = MarsLanguage::keyword("class");
    EXPECT(r.has(klass));
    EXPECT_EQUAL(r.values(klass).at(0), "od");
    r.setValue(klass, "rd");
    EXPECT_EQUAL(r["class"], "rd");
    EXPECT_EQUAL(r.params().size(), 2u);

    // the parameters keep the order they were added in
    EXPECT_EQUAL(r.params(), (std::vector<std::string>{"custom", "class"}));

    r.erase(klass);
    EXPECT(!r.has("class"));
    EXPECT_EQUAL(r.params(), (std::vector<std::string>{"custom"}));
}

CASE("test_request_typed_parameters") {
    MarsRequest r = MarsRequest::parse(
        "retrieve,class=od,expver=0079,stream=enfh,date=20240729,time=00/12,type=fcmean,levtype=sfc,step=24,number=1/"
        "to/2,param=mucin/mucape/tprate,area=12/13/14/15,grid=.1/.1");

    // expanded parameters are typed, and are found by keyword or by name, whatever the case of the name
    Keyword klass      = MarsLanguage::keyword("class");
    const Parameter* p = r.find("class");
    EXPECT(p != nullptr);
    EXPECT(p->typed());
    EXPECT_EQUAL(p->id(), klass);
    EXPECT(r.find(klass) == p);
    EXPECT(r.find("CLASS") == p);
    EXPECT(r.has("Class"));

    EXPECT_EQUAL(r.countValues("param"), 3u);
    EXPECT_EQUAL(r.countValues("number"), 2u);
    EXPECT(r.find("param")->multiple());
    EXPECT(!r.find("class")->multiple());

    // modifying the values keeps the type
    r.setValue("step", "48");
    EXPECT(r.find("step")->typed());
    EXPECT_EQUAL(r["step"], "48");

    // an expanded request is a request like any other
    r.setValue("custom", "x");
    EXPECT(!r.find("custom")->typed());
    r.erase(klass);
    EXPECT(!r.has("class"));
    EXPECT(r.has("custom"));
}

CASE("test_request_copy_and_move") {
    MarsRequest a("retrieve");
    a.setValue("class", "od");

    // copies are independent
    MarsRequest b(a);
    b.setValue("class", "rd");
    b.setValue("type", "an");
    EXPECT_EQUAL(a["class"], "od");
    EXPECT(!a.has("type"));
    EXPECT_EQUAL(b["class"], "rd");

    MarsRequest c(std::move(b));
    EXPECT_EQUAL(c["class"], "rd");
    EXPECT_EQUAL(c["type"], "an");
    EXPECT_EQUAL(c.verb(), "retrieve");

    // and a moved-from request can be used again
    b = a;
    EXPECT_EQUAL(b["class"], "od");

    // a vector of requests can grow: the requests are moved (and must not be lost)
    std::vector<MarsRequest> requests;
    for (int i = 0; i < 100; ++i) {
        MarsRequest r("retrieve");
        r.setValue("step", i);
        requests.push_back(std::move(r));
    }
    for (int i = 0; i < 100; ++i) {
        EXPECT_EQUAL(requests[i]["step"], std::to_string(i));
    }
}

CASE("test_request_stream_validate") {
    MarsRequest r("retrieve");
    r.setValue("class", "od");
    r.setValue("type", "an");

    eckit::Buffer buffer(4096);
    eckit::ResizableMemoryStream out(buffer);
    out << r;

    // validating the request types the parameters
    eckit::MemoryStream in(buffer);
    MarsRequest decoded(in, true);
    EXPECT(decoded.find("class")->typed());
    EXPECT(decoded.find("type")->typed());
    EXPECT_EQUAL(decoded["class"], "od");

    // ... and rejects the keywords that are not in the language
    MarsRequest custom("retrieve");
    custom.setValue("not_a_keyword", "x");
    eckit::ResizableMemoryStream out2(buffer);
    out2 << custom;
    eckit::MemoryStream in2(buffer);
    EXPECT_THROWS_AS(MarsRequest(in2, true), eckit::UserError);

    // the case of the names can be normalised
    MarsRequest upper("RETRIEVE");
    upper.setValue("CLASS", "od");
    eckit::ResizableMemoryStream out3(buffer);
    out3 << upper;
    eckit::MemoryStream in3(buffer);
    MarsRequest lower(in3, false, true);
    EXPECT_EQUAL(lower.verb(), "retrieve");
    EXPECT(lower.find("class") != nullptr);
}

CASE("test_raw_request_matches") {
    MarsRequest r("retrieve");
    r.setValue("class", "od");
    r.values("param", std::vector<std::string>{"t", "z"});

    MarsRequest filter("retrieve");
    filter.values("param", std::vector<std::string>{"z"});
    EXPECT(r.matches(filter));

    MarsRequest other("retrieve");
    other.values("param", std::vector<std::string>{"u"});
    EXPECT(!r.matches(other));

    MarsRequest missing("retrieve");
    missing.setValue("levtype", "sfc");
    EXPECT(!r.matches(missing));
}

CASE("test_language_verbs") {
    // verbs are case insensitive, and aliases are resolved
    EXPECT_EQUAL(&MarsLanguage::get("RETRIEVE"), &MarsLanguage::get("retrieve"));
    EXPECT_EQUAL(&MarsLanguage::get("ret"), &MarsLanguage::get("retrieve"));

    // an unknown verb is an error of the user
    EXPECT_THROWS_AS(MarsLanguage::get("not_a_verb"), eckit::UserError);
    EXPECT_THROWS_AS(MarsLanguage::expandVerb("not_a_verb"), eckit::UserError);
}

CASE("test_request_no_implicit_conversions") {
    // a verb (an integral) or a string must not silently become a request
    EXPECT(!(std::is_convertible_v<Verb, MarsRequest>));
    EXPECT(!(std::is_convertible_v<int, MarsRequest>));
    EXPECT(!(std::is_convertible_v<std::string, MarsRequest>));
    EXPECT(!(std::is_convertible_v<const char*, MarsRequest>));

    EXPECT((std::is_constructible_v<MarsRequest, Verb>));
    EXPECT((std::is_constructible_v<MarsRequest, std::string>));

    Verb retrieve = MarsLanguage::verb("retrieve");
    EXPECT_EQUAL(MarsRequest{retrieve}.verb(), "retrieve");
    EXPECT_EQUAL(MarsRequest{"retrieve"}.verb(), "retrieve");
}

//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test


int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
