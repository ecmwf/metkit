/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_MetFile.cc
/// @date   Jan 2016
/// @author Florian Rathgeber

#include "eckit/types/Date.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/hypercube/HyperCube.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

CASE( "test_metkit_hypercube" ) {
    const char* text = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=500,param=138";
    MarsRequest r = MarsRequest::parse(text);

    metkit::hypercube::HyperCube cube(r);

    EXPECT(cube.contains(r));
    EXPECT(cube.size() == 1);
    EXPECT(cube.request().size() == 1);
    EXPECT(!(r<*cube.request().begin()));
    EXPECT(!(*cube.request().begin()<r));
}

CASE( "test_metkit_hypercube_subset" ) {
    const char* text = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=500/600,param=138";
    MarsRequest r = MarsRequest::parse(text);

    metkit::hypercube::HyperCube cube(r);
    EXPECT(cube.size() == 2);
    EXPECT(cube.count() == 2);
    EXPECT(cube.request().size() == 1);
    EXPECT(!(r<*cube.request().begin()));
    EXPECT(!(*cube.request().begin()<r));

    const char* text500 = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=500,param=138";
    MarsRequest r500 = MarsRequest::parse(text500);
    const char* text600 = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=600,param=138";
    MarsRequest r600 = MarsRequest::parse(text600);

    EXPECT_THROWS(cube.contains(r));
    EXPECT(cube.contains(r500));
    EXPECT(cube.contains(r600));

    cube.clear(r500);

    EXPECT(!cube.contains(r500));
    EXPECT(cube.size() == 2);
    EXPECT(cube.count() == 1);

    EXPECT(cube.request().size() == 1);
    EXPECT(!(r600<*cube.request().begin()));
    EXPECT(!(*cube.request().begin()<r600));

    cube.clear(r600);

    EXPECT(cube.count() == 0);
}

CASE( "test_metkit_hypercube_request" ) {
    const char* text = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=500/600,param=138/155";
    MarsRequest r = MarsRequest::parse(text);

    metkit::hypercube::HyperCube cube(r);
    EXPECT(cube.size() == 4);
    EXPECT(cube.count() == 4);
    EXPECT(cube.request().size() == 1);
    EXPECT(!(r<*cube.request().begin()));
    EXPECT(!(*cube.request().begin()<r));

    const char* text500 = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=500,param=138";
    MarsRequest r500 = MarsRequest::parse(text500);
    const char* text600 = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=600,param=138";
    MarsRequest r600 = MarsRequest::parse(text600);

    EXPECT_THROWS(cube.contains(r));
    EXPECT(cube.contains(r500));
    EXPECT(cube.contains(r600));

    cube.clear(r500);

    EXPECT(!cube.contains(r500));
    EXPECT(cube.size() == 4);
    EXPECT(cube.count() == 3);

    EXPECT(cube.request().size() == 2);

    cube.clear(r600);

    EXPECT(cube.count() == 2);
    EXPECT(cube.request().size() == 1);

    const char* text155 = "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist=500/600,param=155";
    MarsRequest r155 = MarsRequest::parse(text155);
    EXPECT(!(r155<*cube.request().begin()));
    EXPECT(!(*cube.request().begin()<r155));

}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
