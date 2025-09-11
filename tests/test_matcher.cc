/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

#include "eckit/testing/Test.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Matcher.h"

using namespace eckit::testing;
using namespace metkit::mars;
namespace metkit::test {

//----------------------------------------------------------------------------------------------------------------------

CASE("match") {

    // match any x***
    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2"});
    req.setValue("stream", "enfo");
    req.setValue("step ", "0");  // step is not in the matcher

    EXPECT_EQUAL(req.count(), 2);
    EXPECT_EQUAL(match_any.match(req), true);
    EXPECT_EQUAL(match_all.match(req), true);

    // -- Test partially matching request

    req = MarsRequest("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2", "3"});  // number=3 does not match
    req.setValue("stream", "enfo");

    EXPECT_EQUAL(req.count(), 3);
    EXPECT_EQUAL(match_any.match(req), true);
    EXPECT_EQUAL(match_all.match(req), false);

    // -- Test request entirely not matching

    req = MarsRequest("retrieve");
    req.setValue("expver", "yyyy");  // expver=yyyy does not match
    req.values("number", {"1", "2"});
    req.setValue("stream", "enfo");

    EXPECT_EQUAL(req.count(), 2);
    EXPECT_EQUAL(match_any.match(req), false);
    EXPECT_EQUAL(match_all.match(req), false);

    // -- Test request missing keys

    req = MarsRequest("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2"});
    // stream is not set
    EXPECT_EQUAL(req.count(), 2);

    bool matchMissing = false;
    EXPECT_EQUAL(match_any.match(req, matchMissing), false);
    EXPECT_EQUAL(match_all.match(req, matchMissing), false);

    matchMissing = true;
    EXPECT_EQUAL(match_any.match(req, matchMissing), true);
    EXPECT_EQUAL(match_all.match(req, matchMissing), true);

    // -- Combinations of the above some of the above

    // Missing key, but also wrong expver.
    req = MarsRequest("retrieve");
    req.setValue("expver", "yyyy");         // yyyy does not match
    req.values("number", {"1", "2", "3"});  // number=3 does not match
    EXPECT_EQUAL(req.count(), 3);

    matchMissing = false;
    EXPECT_EQUAL(match_any.match(req, matchMissing), false);
    EXPECT_EQUAL(match_all.match(req, matchMissing), false);

    matchMissing = true;
    EXPECT_EQUAL(match_any.match(req, matchMissing), false);
    EXPECT_EQUAL(match_all.match(req, matchMissing), false);


    // Missing key, but expver matches, number partially matches
    req = MarsRequest("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2", "3"});

    matchMissing = false;
    EXPECT_EQUAL(match_any.match(req, matchMissing), false);
    EXPECT_EQUAL(match_all.match(req, matchMissing), false);

    matchMissing = true;
    EXPECT_EQUAL(match_any.match(req, matchMissing), true);
    EXPECT_EQUAL(match_all.match(req, matchMissing), false);
}

//----------------------------------------------------------------------------------------------------------------------


}  // namespace metkit::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
