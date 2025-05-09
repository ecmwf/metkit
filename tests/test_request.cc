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
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"type\":\"fcmean\",\"stream\":\"enfh\",\"levtype\":\"sfc\",\"date\":\"20240729\",\"time\":[\"0000\",\"1200\"],\"step\":\"24\",\"expver\":\"0079\",\"number\":[\"1\",\"2\"],\"param\":[\"228236\",\"228235\",\"172228\"],\"domain\":\"g\",\"repres\":\"sh\"}");                         
        }
        {
            std::stringstream ss;
            eckit::JSON array(ss);
            r.json(array, true);
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"type\":\"fcmean\",\"stream\":\"enfh\",\"levtype\":\"sfc\",\"date\":[\"20240729\"],\"time\":[\"0000\",\"1200\"],\"step\":[\"24\"],\"expver\":\"0079\",\"number\":[\"1\",\"2\"],\"param\":[\"228236\",\"228235\",\"172228\"],\"domain\":\"g\",\"repres\":\"sh\"}");
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
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"type\":\"an\",\"stream\":\"wave\",\"levtype\":\"sfc\",\"date\":\"20240729\",\"time\":\"0000\",\"step\":\"24\",\"expver\":\"0001\",\"param\":\"140251\",\"domain\":\"g\",\"repres\":\"sh\"}");
        }
        {
            std::stringstream ss;
            eckit::JSON array(ss);
            r.json(array, true);
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"type\":\"an\",\"stream\":\"wave\",\"levtype\":\"sfc\",\"date\":[\"20240729\"],\"time\":[\"0000\"],\"step\":[\"24\"],\"expver\":\"0001\",\"param\":[\"140251\"],\"domain\":\"g\",\"repres\":\"sh\"}");
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
        "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/to/137,levtype=ml,number=-1,param=z,process=local,step=000,stream=scda,time=18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(1, r.count());
    }
    {
        const char* text =
        "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/to/137,levtype=ml,number=-1,param=z/t,process=local,step=000,stream=scda,time=18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(138, r.count());
    }
    {
        const char* text =
        "retrieve,accuracy=16,class=od,date=20230810,expver=1,levelist=1/to/137,levtype=ml,number=-1,param=22/127/128/129/152/u/v,process=local,step=000,stream=scda,time=18,type=an,target=reference.data";
        MarsRequest r = MarsRequest::parse(text);
        EXPECT_EQUAL(279, r.count());
    }
}


//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
