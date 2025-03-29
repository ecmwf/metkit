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
#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsRequest.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit::mars::test {

//-----------------------------------------------------------------------------

CASE("test_metkit_hypercube") {
    const char* text =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500,param=138";
    MarsRequest r = MarsRequest::parse(text);

    metkit::hypercube::HyperCube cube(r);

    EXPECT(cube.contains(r));
    EXPECT(cube.size() == 1);
    EXPECT(cube.vacantRequests().size() == 1);
    EXPECT_EQUAL("sh", r.values("repres").at(0));
    r.unsetValues("repres");
    EXPECT(!(r < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r));
}

CASE("test_metkit_hypercube_subset") {
    const char* text =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500/600,param=138";
    MarsRequest r = MarsRequest::parse(text);

    metkit::hypercube::HyperCube cube(r);
    EXPECT(cube.size() == 2);
    EXPECT(cube.countVacant() == 2);
    EXPECT(cube.vacantRequests().size() == 1);
    EXPECT_EQUAL("sh", r.values("repres").at(0));
    r.unsetValues("repres");
    EXPECT(!(r < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r));

    const char* text500 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500,param=138";
    MarsRequest r500 = MarsRequest::parse(text500);
    EXPECT_EQUAL("sh", r500.values("repres").at(0));
    r500.unsetValues("repres");

    const char* text600 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "600,param=138";
    MarsRequest r600 = MarsRequest::parse(text600);
    EXPECT_EQUAL("sh", r600.values("repres").at(0));
    r600.unsetValues("repres");

    EXPECT_THROWS(cube.contains(r));
    EXPECT(cube.contains(r500));
    EXPECT(cube.contains(r600));

    cube.clear(r500);

    EXPECT(!cube.contains(r500));
    EXPECT(cube.size() == 2);
    EXPECT(cube.countVacant() == 1);

    EXPECT(cube.vacantRequests().size() == 1);
    EXPECT(!(r600 < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r600));

    cube.clear(r600);

    EXPECT(cube.countVacant() == 0);
}

CASE("test_metkit_hypercube_request") {
    const char* text =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500/600,param=138/155";
    MarsRequest r = MarsRequest::parse(text);

    metkit::hypercube::HyperCube cube(r);
    EXPECT(cube.size() == 4);
    EXPECT(cube.countVacant() == 4);
    EXPECT(cube.vacantRequests().size() == 1);
    EXPECT_EQUAL("sh", r.values("repres").at(0));
    r.unsetValues("repres");
    EXPECT(!(r < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r));

    const char* text500 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500,param=138";
    MarsRequest r500 = MarsRequest::parse(text500);
    EXPECT_EQUAL("sh", r500.values("repres").at(0));
    r500.unsetValues("repres");

    const char* text600 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "600,param=138";
    MarsRequest r600 = MarsRequest::parse(text600);
    EXPECT_EQUAL("sh", r600.values("repres").at(0));
    r600.unsetValues("repres");

    EXPECT_THROWS(cube.contains(r));
    EXPECT(cube.contains(r500));
    EXPECT(cube.contains(r600));

    cube.clear(r500);

    EXPECT(!cube.contains(r500));
    EXPECT(cube.size() == 4);
    EXPECT(cube.countVacant() == 3);

    EXPECT(cube.vacantRequests().size() == 2);

    cube.clear(r600);

    EXPECT(cube.countVacant() == 2);
    EXPECT(cube.vacantRequests().size() == 1);

    const char* text155 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500/600,param=155";
    MarsRequest r155 = MarsRequest::parse(text155);
    EXPECT_EQUAL("sh", r155.values("repres").at(0));
    r155.unsetValues("repres");
    EXPECT(!(r155 < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r155));
}

//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
