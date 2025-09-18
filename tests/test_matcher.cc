/*
 * (C) Copyright 2025- ECMWF.
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

// ----------------------------------------------------------------------------------------------------------------------

CASE("parse string with spaces") {
    std::map<std::string, eckit::Regex> map =
        parseMatchString("expver=(x[0-9a-z]{3}), number = (1|2) , stream=  ^enfo$  ");
    EXPECT(map.size() == 3);
    EXPECT(map.find("expver") != map.end());
    EXPECT(map.find("number") != map.end());
    EXPECT(map.find("stream") != map.end());
}

CASE("parse string errors") {
    // Check basic parsing
    EXPECT_THROWS_AS(parseMatchString(""), eckit::BadValue);
    EXPECT_THROWS_AS(parseMatchString("expver"), eckit::BadValue);
    EXPECT_THROWS_AS(parseMatchString("=expver"), eckit::BadValue);
    EXPECT_THROWS_AS(parseMatchString("expver="), eckit::BadValue);
    EXPECT_THROWS_AS(parseMatchString("expver,number=(1|2)"), eckit::BadValue);
    EXPECT_THROWS_AS(parseMatchString("number=(1|2),number=(3|4)"), eckit::BadValue);
}

CASE("match basic") {

    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2"});
    req.setValue("stream", "enfo");
    req.setValue("step ", "0");  // step is not in the matcher. This should have no effect on matching

    EXPECT_EQUAL(req.count(), 2);
    EXPECT_EQUAL(match_any.match(req), true);
    EXPECT_EQUAL(match_all.match(req), true);
}

CASE("partially matching request") {

    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2", "3"});  // number=3 does not match, the others do
    req.setValue("stream", "enfo");

    EXPECT_EQUAL(req.count(), 3);
    EXPECT_EQUAL(match_any.match(req), true);
    EXPECT_EQUAL(match_all.match(req), false);
}

CASE("request entirely not matching") {

    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");
    req.setValue("expver", "yyyy");  // expver=yyyy does not match
    req.values("number", {"1", "2"});
    req.setValue("stream", "enfo");

    EXPECT_EQUAL(req.count(), 2);
    EXPECT_EQUAL(match_any.match(req), false);
    EXPECT_EQUAL(match_all.match(req), false);
}

CASE("match with missing keys") {

    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2"});
    // stream is not set: matching depends on MatchMissingPolicy
    EXPECT_EQUAL(req.count(), 2);

    EXPECT_EQUAL(match_any.match(req, Matcher::DontMatchOnMissing), false);
    EXPECT_EQUAL(match_all.match(req, Matcher::DontMatchOnMissing), false);

    EXPECT_EQUAL(match_any.match(req, Matcher::MatchOnMissing), true);
    EXPECT_EQUAL(match_all.match(req, Matcher::MatchOnMissing), true);
}

CASE("match missing key and wrong values") {
    // -- Combinations of the above some of the above

    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");
    req.setValue("expver", "yyyy");         // yyyy does not match
    req.values("number", {"1", "2", "3"});  // number=3 does not match
    EXPECT_EQUAL(req.count(), 3);

    EXPECT_EQUAL(match_any.match(req, Matcher::DontMatchOnMissing), false);
    EXPECT_EQUAL(match_all.match(req, Matcher::DontMatchOnMissing), false);

    EXPECT_EQUAL(match_any.match(req, Matcher::MatchOnMissing), false);
    EXPECT_EQUAL(match_all.match(req, Matcher::MatchOnMissing), false);
}

CASE("match missing key but some values match") {
    // Missing key, but expver matches, number partially matches

    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");
    req.setValue("expver", "xxxx");
    req.values("number", {"1", "2", "3"});

    EXPECT_EQUAL(match_any.match(req, Matcher::DontMatchOnMissing), false);
    EXPECT_EQUAL(match_all.match(req, Matcher::DontMatchOnMissing), false);

    EXPECT_EQUAL(match_any.match(req, Matcher::MatchOnMissing), true);
    EXPECT_EQUAL(match_all.match(req, Matcher::MatchOnMissing), false);
}

CASE("match empty request") {
    // Testing that this does not raise an exception
    Matcher match_any("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::Any);
    Matcher match_all("expver=(x[0-9a-z]{3}),number=(1|2),stream=^enfo$", Matcher::Policy::All);

    MarsRequest req("retrieve");

    EXPECT_EQUAL(match_any.match(req, Matcher::DontMatchOnMissing), false);
    EXPECT_EQUAL(match_all.match(req, Matcher::DontMatchOnMissing), false);

    EXPECT_EQUAL(match_any.match(req, Matcher::MatchOnMissing), true);
    EXPECT_EQUAL(match_all.match(req, Matcher::MatchOnMissing), true);
}

// ----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::test

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
