/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

#include "metkit/mars2grib/testing-utils/strip/Envelope.h"
#include "metkit/mars2grib/testing-utils/strip/Plan.h"

using namespace eckit::testing;
using namespace metkit::mars2grib::testing_utils::strip;

namespace metkit::test {

namespace {

/// An envelope as the owner rank receives it (no classification here: fields set by hand).
Envelope envelope(const std::string& mars, const std::string& misc = "{}", bool firstEmission = true,
                  const std::string& shape = "Instant") {
    Envelope e;
    e.record        = R"({"mars":)" + mars + R"(,"misc":)" + misc + R"(,"opt":{},"out":{"operations":[]}})";
    e.route         = 42;
    e.anchorType    = "ForecastAnalysis";
    e.domainType    = "ForecastDomain";
    e.shapeType     = shape;
    e.firstEmission = firstEmission;
    e.classified    = true;
    return e;
}

Plan plan(const std::string& yaml) {
    const eckit::LocalConfiguration config{eckit::YAMLConfiguration{yaml}};
    return Plan{config.getSubConfigurations("actions")};
}

/// Indices of the envelopes the plan keeps, processed in order.
std::vector<int> kept(Plan& p, const std::vector<Envelope>& envelopes) {
    std::vector<int> out;
    for (std::size_t i = 0; i < envelopes.size(); ++i) {
        if (p.keep(TestView::parse(envelopes[i]))) {
            out.push_back(static_cast<int>(i));
        }
    }
    return out;
}

}  // namespace

CASE("reduce keeps one ensemble member and the control forecast") {
    Plan p = plan(R"({actions: [{type: reduce, match: "type=pf", vary: [number], keep: 1}]})");
    const std::vector<Envelope> in{
        envelope(R"({"type":"pf","number":3,"param":130})"), envelope(R"({"type":"pf","number":1,"param":130})"),
        envelope(R"({"type":"cf","param":130})"), envelope(R"({"type":"pf","number":2,"param":167})"),
        envelope(R"({"type":"pf","number":5,"param":167})")};
    EXPECT(kept(p, in) == (std::vector<int>{0, 2, 3}));
    EXPECT_EQUAL(p.actions()[0]->dropped(), 2);
}

CASE("reduce keeps one model level and two pressure levels around 1 hPa") {
    Plan p = plan(R"({actions: [
        {type: reduce, match: "levtype=ml", vary: [levelist], keep: 1},
        {type: reduce, match: "levtype=pl,levelist=0/to/0.9999", vary: [levelist], keep: 1},
        {type: reduce, match: "levtype=pl,levelist=1/to/1e9", vary: [levelist], keep: 1}]})");
    const std::vector<Envelope> in{envelope(R"({"levtype":"ml","levelist":10,"param":130})"),
                                   envelope(R"({"levtype":"ml","levelist":1,"param":130})"),
                                   envelope(R"({"levtype":"pl","levelist":0.5,"param":130})"),
                                   envelope(R"({"levtype":"pl","levelist":0.7,"param":130})"),
                                   envelope(R"({"levtype":"pl","levelist":850,"param":130})"),
                                   envelope(R"({"levtype":"pl","levelist":500,"param":130})")};
    EXPECT(kept(p, in) == (std::vector<int>{0, 2, 4}));
}

CASE("reduce keeps one frequency/direction pair of wave spectra") {
    Plan p = plan(R"({actions: [{type: reduce, match: "frequency=1/to/1e9", vary: [frequency, direction], keep: 1}]})");
    const std::vector<Envelope> in{envelope(R"({"param":251,"frequency":3,"direction":1})"),
                                   envelope(R"({"param":251,"frequency":3,"direction":2})"),
                                   envelope(R"({"param":251,"frequency":1,"direction":1})"),
                                   envelope(R"({"param":130})")};
    EXPECT(kept(p, in) == (std::vector<int>{0, 3}));
}

CASE("first emission and one step per time representation") {
    Plan p = plan(R"({actions: [{type: first-emission},
                                {type: reduce, vary: [step], keep: 1, group-by: [mars, time-representation]}]})");
    const std::vector<Envelope> in{envelope(R"({"param":228,"step":12})", "{}", false, "IFSStandardSingleLoop"),
                                   envelope(R"({"param":228,"step":6})", "{}", true, "IFSStandardSingleLoop"),
                                   envelope(R"({"param":228,"step":18})", "{}", true, "IFSStandardSingleLoop"),
                                   envelope(R"({"param":228,"step":24})", "{}", true, "SeasonalSingleLoop")};
    EXPECT(kept(p, in) == (std::vector<int>{1, 3}));
}

CASE("a kept combination keeps records differing only in misc unless misc is in the group") {
    Plan byMars = plan(R"({actions: [{type: reduce, match: "type=pf", vary: [number], keep: 1}]})");
    const std::vector<Envelope> in{envelope(R"({"type":"pf","number":1})", R"({"a":1})"),
                                   envelope(R"({"type":"pf","number":1})", R"({"a":2})"),
                                   envelope(R"({"type":"pf","number":2})", R"({"a":3})")};
    EXPECT(kept(byMars, in) == (std::vector<int>{0, 1}));

    Plan withMisc =
        plan(R"({actions: [{type: reduce, match: "type=pf", vary: [number], keep: 1, group-by: [mars, misc]}]})");
    EXPECT(kept(withMisc, in) == (std::vector<int>{0, 1, 2}));
}

CASE("select and deduplicate") {
    Plan p = plan(R"({actions: [{type: select, exclude: "class=rd"}, {type: deduplicate}]})");
    const std::vector<Envelope> in{envelope(R"({"class":"od","param":130})"), envelope(R"({"param":130,"class":"od"})"),
                                   envelope(R"({"class":"rd","param":130})"),
                                   envelope(R"({"class":"od","param":167})")};
    EXPECT(kept(p, in) == (std::vector<int>{0, 3}));
}

CASE("envelopes survive the exchange buffer") {
    std::vector<char> buffer;
    const Envelope a = envelope(R"({"param":130})", "{}", false, "Instant");
    const Envelope b = envelope(R"({"param":167})");
    appendEnvelope(buffer, a);
    appendEnvelope(buffer, b);
    std::size_t position = 0;
    Envelope out;
    EXPECT(readEnvelope(buffer, position, out));
    EXPECT_EQUAL(out.record, a.record);
    EXPECT(!out.firstEmission);
    EXPECT_EQUAL(out.timeRepresentation(), a.timeRepresentation());
    EXPECT(readEnvelope(buffer, position, out));
    EXPECT_EQUAL(out.record, b.record);
    EXPECT(!readEnvelope(buffer, position, out));
}

CASE("invalid plans are rejected") {
    EXPECT_THROWS_AS(plan(R"({actions: [{type: nonsense}]})"), eckit::UserError);
    EXPECT_THROWS_AS(plan(R"({actions: [{type: reduce, keep: 1}]})"), eckit::UserError);
    EXPECT_THROWS_AS(plan(R"({actions: [{type: reduce, vary: [number], keep: 0}]})"), eckit::UserError);
    EXPECT_THROWS_AS(plan(R"({actions: [{type: reduce, vary: [number], group-by: [bogus]}]})"), eckit::UserError);
    EXPECT_THROWS_AS(plan(R"({actions: [{type: select}]})"), eckit::UserError);
}

}  // namespace metkit::test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
