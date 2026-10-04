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

#include "eckit/io/Buffer.h"
#include "eckit/log/JSON.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/serialisation/ResizableMemoryStream.h"
#include "eckit/types/Date.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"

#include "eckit/testing/Test.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

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

CASE("test_validated_request_unknown_names") {
    MarsRequest r(MarsLanguage::verb("retrieve"));
    r.setValue("class", "od");

    // queries about a name that is not a keyword are not errors
    EXPECT(r.has("class"));
    EXPECT(!r.has("not_a_keyword"));
    EXPECT(r.find("not_a_keyword") == nullptr);
    EXPECT_EQUAL(r.countValues("not_a_keyword"), 0u);
    EXPECT(r.values("not_a_keyword", true).empty());
    EXPECT_NO_THROW(r.unsetValues("not_a_keyword"));

    // ... but asking for the values of a missing parameter, or setting an unknown keyword, is the user's error
    EXPECT_THROWS_AS(r.values("not_a_keyword"), eckit::UserError);
    EXPECT_THROWS_AS(r.setValue("not_a_keyword", "x"), eckit::UserError);
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

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
