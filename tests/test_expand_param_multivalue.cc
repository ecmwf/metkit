/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_expand_param_multivalue.cc
/// @date   Aug 2026
/// @author Emanuele Danovaro

#include <cstdlib>
#include <cstring>
#include <fstream>

#include "metkit/mars/MarsRequest.h"

#include "eckit/config/Resource.h"
#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit::mars::test {

CASE("test_metkit_expand_param") {
    ::setenv("METKIT_MULTI_PARAM_VALUES", "1", 1);
    ::setenv("METKIT_PRECOMPUTED_PARAM", "0", 1);

    bool multiParamValues = eckit::Resource<bool>("metkitMultiParamValues;$METKIT_MULTI_PARAM_VALUES", false);
    bool precomputedParam = eckit::Resource<bool>("metkitPrecomputedParam;$METKIT_PRECOMPUTED_PARAM", true);

    std::cout << "multiParamValues: " << multiParamValues << std::endl;
    std::cout << "precomputedParam: " << precomputedParam << std::endl;

    ASSERT(multiParamValues && !precomputedParam);
    {
        const char* text =
            "retrieve,class=od,date=20240723,domain=g,expver=0079,levtype=sfc,param=asn/cp/lsp/sf/tcc/"
            "tp,step=0,stream=oper,time=0000,type=fc";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 6);

        EXPECT_EQUAL(params[0], "228032|32");
        EXPECT_EQUAL(params[1], "228143|143");
        EXPECT_EQUAL(params[2], "3062|142");
        EXPECT_EQUAL(params[3], "228144|144");
        EXPECT_EQUAL(params[4], "228164|164");
        EXPECT_EQUAL(params[5], "228228|228");
    }
}
//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
