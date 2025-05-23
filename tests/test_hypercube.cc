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

#include <algorithm>

#include "eckit/testing/Test.h"

#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsRequest.h"

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
    EXPECT(!(r < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r));

    const char* text500 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500,param=138";
    MarsRequest r500 = MarsRequest::parse(text500);

    const char* text600 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "600,param=138";
    MarsRequest r600 = MarsRequest::parse(text600);

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
    EXPECT(!(r < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r));

    const char* text500 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "500,param=138";
    MarsRequest r500 = MarsRequest::parse(text500);

    const char* text600 =
        "retrieve,class=rd,type=an,stream=oper,levtype=pl,date=20191110,time=0000,step=0,expver=xxxy,domain=g,levelist="
        "600,param=138";
    MarsRequest r600 = MarsRequest::parse(text600);

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
    EXPECT(!(r155 < *cube.vacantRequests().begin()));
    EXPECT(!(*cube.vacantRequests().begin() < r155));
}

CASE("test_metkit_hypercube_request METK-132") {
    std::vector<std::string> keys = {"levelist", "param"};

    MarsRequest r = MarsRequest::parse("retrieve,levelist=2/4/1/3/5,param=228038/235094/235077/235078");
    metkit::hypercube::HyperCube cube{r};

    // unset levelist 5 for params 235077/235094
    cube.clear(MarsRequest::parse("retrieve,levelist=5,param=235077"));
    cube.clear(MarsRequest::parse("retrieve,levelist=5,param=235094"));

    std::vector<MarsRequest> expected_flat_requests = {
        MarsRequest::parse("retrieve,levelist=1,param=228038"), MarsRequest::parse("retrieve,levelist=2,param=228038"),
        MarsRequest::parse("retrieve,levelist=3,param=228038"), MarsRequest::parse("retrieve,levelist=4,param=228038"),
        MarsRequest::parse("retrieve,levelist=5,param=228038"), MarsRequest::parse("retrieve,levelist=1,param=235077"),
        MarsRequest::parse("retrieve,levelist=2,param=235077"), MarsRequest::parse("retrieve,levelist=3,param=235077"),
        MarsRequest::parse("retrieve,levelist=4,param=235077"), MarsRequest::parse("retrieve,levelist=1,param=235094"),
        MarsRequest::parse("retrieve,levelist=2,param=235094"), MarsRequest::parse("retrieve,levelist=3,param=235094"),
        MarsRequest::parse("retrieve,levelist=4,param=235094"), MarsRequest::parse("retrieve,levelist=1,param=235078"),
        MarsRequest::parse("retrieve,levelist=2,param=235078"), MarsRequest::parse("retrieve,levelist=3,param=235078"),
        MarsRequest::parse("retrieve,levelist=4,param=235078"), MarsRequest::parse("retrieve,levelist=5,param=235078"),
        // These two requests we have removed from the cube:
        // MarsRequest::parse("retrieve,levelist=5,param=235077"),
        // MarsRequest::parse("retrieve,levelist=5,param=235094"),
    };

    auto dense_requests = cube.vacantRequests();
    EXPECT_EQUAL(dense_requests.size(), 2);

    std::vector<MarsRequest> flattened_requests;
    for (const auto& req : cube.vacantRequests()) {  // surely this should be requests, not vacantRequests... anyway...
        auto flattened = req.split(keys);
        for (const auto& f : flattened) {
            flattened_requests.push_back(f);
        }
    }

    EXPECT_EQUAL(flattened_requests.size(), 18);
    EXPECT_EQUAL(flattened_requests.size(), expected_flat_requests.size());

    // compare both vectors
    std::sort(flattened_requests.begin(), flattened_requests.end());
    std::sort(expected_flat_requests.begin(), expected_flat_requests.end());
    for (size_t i = 0; i < expected_flat_requests.size(); ++i) {
        EXPECT_EQUAL(flattened_requests[i].values("levelist"), expected_flat_requests[i].values("levelist"));
        EXPECT_EQUAL(flattened_requests[i].values("param"), expected_flat_requests[i].values("param"));
    }
}

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
