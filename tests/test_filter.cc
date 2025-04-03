/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_filter.cc
/// @date   Apr 2025
/// @author Emanuele Danovaro

#include <utility>

#include "eckit/types/Date.h"
#include "eckit/utils/StringTools.h"
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"

#include "eckit/testing/Test.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit::testing;

namespace metkit::mars::test {

namespace {
using ExpectedOutput = std::map<std::string, std::vector<std::string>>;
}

//-----------------------------------------------------------------------------


void filter(MarsRequest& r, const MarsRequest& f, const ExpectedOutput& expected,
            const std::vector<long> dates) {

    std::cout << r << std::endl;
    std::cout << f << std::endl;

    r.filter(f);

    std::cout << r << std::endl;

    for (const auto& e : expected) {
        if (!r.has(e.first)) {
            std::cerr << eckit::Colour::red << "Missing keyword: " << e.first << eckit::Colour::reset << std::endl;
        }
        EXPECT(r.has(e.first));
        auto vv = r.values(e.first);
        if (e.first != "date") {  // dates are verified at a later stage
            EXPECT_EQUAL(e.second.size(), vv.size());
        }
        for (int i = 0; i < vv.size(); i++) {
            if (e.first == "grid") {
                EXPECT_EQUAL(eckit::StringTools::upper(e.second.at(i)), vv.at(i));
            }
            else {
                EXPECT_EQUAL(e.second.at(i), vv.at(i));
            }
        }
    }
    if (dates.size() > 0) {
        EXPECT(r.has("date"));
        auto dd = r.values("date");
        EXPECT_EQUAL(dates.size(), dd.size());
        for (int i = 0; i < dates.size(); i++) {
            long d = dates.at(i);
            if (d < 0) {
                eckit::Date day(d);
                d = day.yyyymmdd();
            }
            EXPECT_EQUAL(std::to_string(d), dd.at(i));
        }
    }
}

void filter(const std::string& text, const std::string& filter_text, const ExpectedOutput& expected, std::vector<long> dates,
            bool strict = false) {
    MarsRequest r = MarsRequest::parse(text, strict);
    std::string f_text = "filter," + filter_text;
    std::istringstream in(f_text);
    metkit::mars::MarsParser parser(in);
    std::vector<metkit::mars::MarsParsedRequest> f = parser.parse();
    ASSERT(f.size() == 1);
    filter(r, f[0], expected, std::move(dates));
}

void expand(const std::string& text, const std::string& filter_text, const std::string& expected, bool strict = false, std::vector<long> dates = {}) {
    ExpectedOutput out;
    eckit::Tokenizer c(",");
    eckit::Tokenizer e("=");
    eckit::Tokenizer s("/");
    eckit::StringList tokens;
    c(expected, tokens);
    std::string verb;
    for (const auto& t : tokens) {
        if (verb.empty()) {
            verb = eckit::StringTools::lower(t);
            continue;
        }
        auto tt = eckit::StringTools::trim(t);
        eckit::StringList kv;
        e(tt, kv);
        EXPECT_EQUAL(2, kv.size());
        auto key = eckit::StringTools::lower(eckit::StringTools::trim(kv[0]));
        if (key == "date") {
            EXPECT_EQUAL(0, dates.size());
        }
        eckit::StringList vals;
        s(kv[1], vals);
        std::vector<std::string> vv;
        for (auto v : vals) {
            auto val = eckit::StringTools::trim(v);
            if (key != "source" && key != "target") {
                val = eckit::StringTools::lower(val);
            }
            if (key == "date") {
                dates.push_back(std::stol(val));
            }
            else {
                vv.push_back(val);
            }
        }
        if (key != "date") {
            out.emplace(key, vv);
        }
    }
    MarsRequest r = MarsRequest::parse(text, strict);
    std::string f_text = "filter," + filter_text;
    std::istringstream in(f_text);
    metkit::mars::MarsParser parser(in);
    std::vector<metkit::mars::MarsParsedRequest> f = parser.parse();
    ASSERT(f.size() == 1);
    filter(r, f[0], out, std::move(dates));
}

CASE("day") {
    const char* text = "ret,date=20250301/to/20250306";
    const char* filter_text = "day=1/3/5/7/9/11/13/15/17/19/21/23/25/27/29/31";
    ExpectedOutput expected{{"class", {"od"}},    {"domain", {"g"}},
                            {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                            {"levtype", {"pl"}},  {"param", {"129"}},
                            {"step", {"0"}},      {"stream", {"oper"}},
                            {"time", {"1200"}},   {"type", {"an"}}};
    filter(text, filter_text, expected, {20250301, 20250303, 20250305});
}

//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
