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
#include <string>
#include <vector>

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsRequest.h"

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit::mars::test {

namespace {

std::vector<std::string> expandParam(const std::string& param) {
    const std::string text =
        "retrieve,class=od,date=20240723,domain=g,expver=0079,levtype=sfc,step=0,stream=oper,time=0000,type=fc,param=" +
        param;
    return MarsRequest::parse(text).values("param");
}

}  // namespace

//-----------------------------------------------------------------------------

CASE("multi-param mode is enabled") {
    // the mode is cached by the first reader, so it must be set before any request is expanded (see main)
    EXPECT(metkit::LibMetkit::isMultiParam());
}

CASE("shortnames expand to all matching paramids") {
    auto params = expandParam("asn/cp/lsp/sf/tcc/tp");

    EXPECT_EQUAL(params.size(), 6);
    EXPECT_EQUAL(params[0], "228032|32");
    EXPECT_EQUAL(params[1], "228143|143");
    EXPECT_EQUAL(params[2], "3062|142");
    EXPECT_EQUAL(params[3], "228144|144");
    EXPECT_EQUAL(params[4], "228164|164");
    EXPECT_EQUAL(params[5], "228228|228");
}

CASE("shortnames with a single paramid are not decorated") {
    auto params = expandParam("2t");

    EXPECT_EQUAL(params.size(), 1);
    EXPECT_EQUAL(params[0], "167");
}

CASE("explicit paramids are left untouched") {
    auto params = expandParam("228032/167");

    EXPECT_EQUAL(params.size(), 2);
    EXPECT_EQUAL(params[0], "228032");
    EXPECT_EQUAL(params[1], "167");
}

CASE("order of the requested values is preserved") {
    auto params = expandParam("2t/asn/tp");

    EXPECT_EQUAL(params.size(), 3);
    EXPECT_EQUAL(params[0], "167");
    EXPECT_EQUAL(params[1], "228032|32");
    EXPECT_EQUAL(params[2], "228228|228");
}

CASE("an expanded request can be expanded again") {
    const auto once = expandParam("asn/tp/2t");

    std::string joined;
    for (const auto& p : once) {
        joined += (joined.empty() ? "" : "/") + p;
    }

    EXPECT_EQUAL(expandParam(joined), once);
}

CASE("invalid alternatives are rejected") {
    EXPECT_THROWS_AS(expandParam("228032|nonexistent"), eckit::UserError);
    EXPECT_THROWS_AS(expandParam("|"), eckit::UserError);
}

//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    ::setenv("METKIT_MULTI_PARAM_VALUES", "1", 1);
    return run_tests(argc, argv);
}
