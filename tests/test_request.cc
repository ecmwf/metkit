// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   test_request.cc
/// @date   Jul 2024
/// @author Emanuele Danovaro

#include <string>
#include <vector>

#include "eckit/log/JSON.h"
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

namespace {

MarsRequest requestWith(const std::string& name, const std::vector<std::string>& values) {
    MarsRequest request;
    request.values(name, values);
    return request;
}

}  // namespace

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

CASE("test_request_data_type_classification") {
    {
        MarsRequest request = requestWith("type", {"ob"});
        EXPECT(request.observation());
        EXPECT(request.is_bufr());
        EXPECT(!request.is_grib());
    }
    {
        MarsRequest request = requestWith("type", {"fb"});
        EXPECT(request.observation());
        EXPECT(request.is_bufr());
    }
    {
        MarsRequest request = requestWith("stream", {"ssmi"});
        EXPECT(request.observation());
    }
    {
        MarsRequest request = requestWith("type", {"ob", "fb"});
        EXPECT(request.observation());
        EXPECT(request.is_bufr());
        EXPECT(!request.is_grib());
    }
    {
        MarsRequest request = requestWith("type", {"im"});
        request.values("stream", std::vector<std::string>{"oper"});
        EXPECT(request.image());
    }
    {
        MarsRequest request = requestWith("type", {"im"});
        request.values("stream", std::vector<std::string>{"ssmi"});
        EXPECT(!request.image());
    }
    {
        MarsRequest request = requestWith("type", {"im"});
        EXPECT(!request.image());
    }
    {
        MarsRequest request = requestWith("type", {"im"});
        request.values("stream", std::vector<std::string>{"oper"});
        EXPECT(request.image());
    }

    EXPECT(requestWith("type", {"sim"}).simulated_image());
    EXPECT(!requestWith("type", {"im"}).simulated_image());
    EXPECT(requestWith("type", {"go"}).gridded_observations());
    EXPECT(!requestWith("type", {"ob"}).gridded_observations());
    {
        MarsRequest request = requestWith("type", {"ai"});
        EXPECT(request.feedback());
        EXPECT(request.is_bufr());
        EXPECT(!request.is_grib());
    }
    {
        MarsRequest request = requestWith("type", {"af"});
        EXPECT(request.feedback());
        EXPECT(request.is_bufr());
    }
    {
        MarsRequest request = requestWith("type", {"ab"});
        EXPECT(request.bias());
        EXPECT(request.is_bufr());
    }
    {
        MarsRequest request = requestWith("type", {"tf"});
        EXPECT(request.track());
        EXPECT(request.is_bufr());
    }

    for (const std::string& type : {"ofb", "mfb", "oai", "sfb", "fsoifb", "fcdfb", "ofa"}) {
        EXPECT(requestWith("type", {type}).is_odb());
    }
    EXPECT(!requestWith("type", {"an"}).is_odb());

    {
        MarsRequest request = requestWith("class", {"ep"});
        request.values("levtype", std::vector<std::string>{"o2d"});
        EXPECT(request.is_ocean_netcdf());
        request.values("levtype", std::vector<std::string>{"o3d"});
        EXPECT(request.is_ocean_netcdf());
    }
    {
        MarsRequest request = requestWith("class", {"od"});
        request.values("levtype", std::vector<std::string>{"o2d"});
        EXPECT(!request.is_ocean_netcdf());
    }
    {
        MarsRequest request = requestWith("class", {"ep"});
        request.values("levtype", std::vector<std::string>{"sfc"});
        EXPECT(!request.is_ocean_netcdf());
    }

    EXPECT(requestWith("type", {"an"}).is_grib());
    EXPECT(MarsRequest().is_grib());
}

CASE("test_request_multiple_type_values") {
    struct Classification {
        bool (MarsRequest::*predicate)() const;
        std::vector<std::string> types;
    };

    const std::vector<Classification> classifications{
        {&MarsRequest::observation, {"ob", "fb"}},
        {&MarsRequest::simulated_image, {"sim", "sim"}},
        {&MarsRequest::gridded_observations, {"go", "go"}},
        {&MarsRequest::feedback, {"ai", "af"}},
        {&MarsRequest::bias, {"ab", "ab"}},
        {&MarsRequest::track, {"tf", "tf"}},
        {&MarsRequest::is_bufr, {"ob", "fb", "ai", "af", "ab", "tf"}},
        {&MarsRequest::is_odb, {"ofb", "mfb", "oai", "sfb", "fsoifb", "fcdfb", "ofa"}},
    };

    for (const auto& classification : classifications) {
        const auto predicate = classification.predicate;
        EXPECT((requestWith("type", classification.types).*predicate)());
        EXPECT(!(MarsRequest().*predicate)());
        EXPECT(!(requestWith("type", {}).*predicate)());
        EXPECT(!(requestWith("type", {classification.types.front(), "an"}).*predicate)());
        EXPECT(!(requestWith("type", {"an", classification.types.front()}).*predicate)());
    }

    const MarsRequest feedback = requestWith("type", {"ai", "af"});
    EXPECT(feedback.is_bufr());
    EXPECT(!feedback.is_grib());
    EXPECT(!feedback.observation());
    const MarsRequest observation = requestWith("type", {"ob", "fb"});
    EXPECT(!observation.feedback());
    const MarsRequest mixed = requestWith("type", {"ob", "ai"});
    EXPECT(!mixed.observation());
    EXPECT(!mixed.feedback());
    EXPECT(mixed.is_bufr());
    EXPECT(!mixed.is_grib());
}

CASE("test_request_multiple_stream_values") {
    const MarsRequest observation = requestWith("stream", {"ssmi", "ssmi"});
    EXPECT(observation.observation());
    EXPECT(observation.is_bufr());
    EXPECT(!observation.is_grib());
    EXPECT(!requestWith("stream", {"ssmi", "oper"}).observation());
    EXPECT(!requestWith("stream", {"oper", "ssmi"}).observation());
    EXPECT(!requestWith("stream", {}).observation());
}

CASE("test_request_multiple_ocean_netcdf_values") {
    MarsRequest request = requestWith("class", {"ep", "ep"});
    request.values("levtype", std::vector<std::string>{"o2d", "o3d"});
    EXPECT(request.is_ocean_netcdf());
    request.values("levtype", std::vector<std::string>{"o2d", "sfc"});
    EXPECT(!request.is_ocean_netcdf());
    request.values("levtype", std::vector<std::string>{});
    EXPECT(!request.is_ocean_netcdf());
    request.unsetValues("levtype");
    EXPECT(!request.is_ocean_netcdf());
    request.values("levtype", std::vector<std::string>{"o2d", "o3d"});
    request.values("class", std::vector<std::string>{"ep", "od"});
    EXPECT(!request.is_ocean_netcdf());
    request.values("class", std::vector<std::string>{});
    EXPECT(!request.is_ocean_netcdf());
    request.unsetValues("class");
    EXPECT(!request.is_ocean_netcdf());
}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
