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

#include "eckit/types/Date.h"
#include "eckit/log/JSON.h"

#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"

#include "eckit/testing/Test.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

CASE( "test_request_json" ) {
    {
        const char* text = "retrieve,class=od,expver=0079,stream=enfh,date=-1,time=00/12,type=fcmean,levtype=sfc,step=24,number=1/to/2,param=mucin/mucape/tprate";
        MarsRequest r = MarsRequest::parse(text);
        {
            std::stringstream ss;
            eckit::JSON plain(ss);
            r.json(plain);
            // std::cout << ss.str() << std::endl;
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"expver\":\"0079\",\"stream\":\"enfh\",\"date\":\"20240729\",\"time\":[\"0000\",\"1200\"],\"type\":\"fcmean\",\"levtype\":\"sfc\",\"step\":\"24\",\"number\":[\"1\",\"2\"],\"param\":[\"228236\",\"228235\",\"172228\"],\"domain\":\"g\"}");
        }
        {
            std::stringstream ss;
            eckit::JSON array(ss);
            r.json(array, true);
            // std::cout << ss.str() << std::endl;
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"expver\":\"0079\",\"stream\":\"enfh\",\"date\":[\"20240729\"],\"time\":[\"0000\",\"1200\"],\"type\":\"fcmean\",\"levtype\":\"sfc\",\"step\":[\"24\"],\"number\":[\"1\",\"2\"],\"param\":[\"228236\",\"228235\",\"172228\"],\"domain\":\"g\"}");
        }
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=wave,date=-1,time=00,type=an,levtype=sfc,step=24,param=2dfd ";
        MarsRequest r = MarsRequest::parse(text);
        {
            std::stringstream ss;
            eckit::JSON plain(ss);
            r.json(plain);
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"expver\":\"0001\",\"stream\":\"wave\",\"date\":\"20240729\",\"time\":\"0000\",\"type\":\"an\",\"levtype\":\"sfc\",\"step\":\"24\",\"param\":\"140251\",\"domain\":\"g\"}");
        }
        {
            std::stringstream ss;
            eckit::JSON array(ss);
            r.json(array, true);
            EXPECT_EQUAL(ss.str(), "{\"class\":\"od\",\"expver\":\"0001\",\"stream\":\"wave\",\"date\":[\"20240729\"],\"time\":[\"0000\"],\"type\":\"an\",\"levtype\":\"sfc\",\"step\":[\"24\"],\"param\":[\"140251\"],\"domain\":\"g\"}");
        }
    }
}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
