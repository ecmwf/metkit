/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_expand.cc
/// @date   Jan 2016
/// @author Florian Rathgeber
/// @author Emanuele Danovaro

#include <utility>

#include "eckit/types/Date.h"
#include "eckit/utils/StringTools.h"
#include "metkit/mars/MarsExpansion.h"
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


void expand(const MarsRequest& r, const std::string& verb, const ExpectedOutput& expected,
            const std::vector<long> dates) {
    // std::cout << "comparing " << r << " with " << expected << " dates " << dates << std::endl;
    EXPECT_EQUAL(verb, r.verb());
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

void expand(const std::string& text, const std::string& verb, const ExpectedOutput& expected, std::vector<long> dates,
            bool strict = false) {
    MarsRequest r = MarsRequest::parse(text, strict);
    expand(r, verb, expected, std::move(dates));
}

void expand(const std::string& text, const std::string& expected, bool strict = false, std::vector<long> dates = {}) {
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
    expand(r, verb, out, std::move(dates));
}

CASE("test_metkit_expand_1") {
    const char* text = "ret,date=-5/to/-1";
    ExpectedOutput expected{{"class", {"od"}},    {"domain", {"g"}},
                            {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                            {"levtype", {"pl"}},  {"param", {"129"}},
                            {"step", {"0"}},      {"stream", {"oper"}},
                            {"time", {"1200"}},   {"type", {"an"}}};
    expand(text, "retrieve", expected, {-5, -4, -3, -2, -1});

    const char* text2 = "ret,date=-5/to/-1.";
    expand(text2, "retrieve", expected, {-5, -4, -3, -2, -1});

    const char* expectedStr =
        "RETRIEVE,CLASS=OD,TYPE=AN,STREAM=OPER,EXPVER=0001,REPRES=SH,LEVTYPE=PL,LEVELIST=1000/850/700/500/400/"
        "300,PARAM=129,TIME=1200,STEP=0,DOMAIN=G";
    expand(text, expectedStr, true, {-5, -4, -3, -2, -1});
}

CASE("test_metkit_expand_2") {
    {
        const char* text = "ret,date=-1";
        ExpectedOutput expected{{"class", {"od"}},    {"domain", {"g"}},
                                {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                {"levtype", {"pl"}},  {"param", {"129"}},
                                {"step", {"0"}},      {"stream", {"oper"}},
                                {"time", {"1200"}},   {"type", {"an"}}};
        expand(text, "retrieve", expected, {-1});
    }
    {
        const char* text = "ret,levtype=ml";
        ExpectedOutput expected{{"class", {"od"}},
                                {"domain", {"g"}},
                                {"expver", {"0001"}},
                                {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                // {"levelist", {"1","2"}}, // keeping the old default, for MARS client tests
                                {"levtype", {"ml"}},
                                {"param", {"129"}},
                                {"step", {"0"}},
                                {"stream", {"oper"}},
                                {"time", {"1200"}},
                                {"type", {"an"}}};
        expand(text, "retrieve", expected, {-1});
    }
}

CASE("test_metkit_expand_3") {
    const char* text = "ret,date=-5/to/-1,grid=n640";
    ExpectedOutput expected{{"class", {"od"}},    {"domain", {"g"}},
                            {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                            {"levtype", {"pl"}},  {"param", {"129"}},
                            {"step", {"0"}},      {"stream", {"oper"}},
                            {"time", {"1200"}},   {"type", {"an"}},
                            {"grid", {"N640"}}};
    expand(text, "retrieve", expected, {-5, -4, -3, -2, -1});
}

CASE("test_metkit_expand_4") {
    const char* text = "ret,date=-5/to/-1,grid=o640";
    ExpectedOutput expected{{"class", {"od"}},    {"domain", {"g"}},
                            {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                            {"levtype", {"pl"}},  {"param", {"129"}},
                            {"step", {"0"}},      {"stream", {"oper"}},
                            {"time", {"1200"}},   {"type", {"an"}},
                            {"grid", {"O640"}}};
    expand(text, "retrieve", expected, {-5, -4, -3, -2, -1});
}

CASE("test_metkit_expand_5") {
    const char* text =
        "retrieve,class=od,date=20050601,diagnostic=1,expver=1,iteration=0,levelist=1,levtype=ml,param=155.129,stream="
        "sens,time=1200,type=sg";
    ExpectedOutput expected{{"class", {"od"}},    {"diagnostic", {"1"}}, {"domain", {"g"}},   {"expver", {"0001"}},
                            {"iteration", {"0"}}, {"levelist", {"1"}},   {"levtype", {"ml"}}, {"param", {"129155"}},
                            {"step", {"0"}},      {"stream", {"sens"}},  {"time", {"1200"}},  {"type", {"sg"}}};
    expand(text, "retrieve", expected, {20050601});
}

CASE("test_metkit_expand_6") {
    const char* text =
        "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param="
        "129,levelist=1/to/31";
    ExpectedOutput expected{{"class", {"rd"}},   {"expver", {"hl1m"}}, {"stream", {"oper"}},
                            {"time", {"0000"}},  {"domain", {"g"}},    {"type", {"fc"}},
                            {"levtype", {"pl"}}, {"step", {"24"}},     {"param", {"129"}}};
    std::vector<std::string> levelist;
    for (int i = 1; i <= 31; i++) {
        levelist.push_back(std::to_string(i));
    }
    expected["levelist"] = levelist;
    expand(text, "retrieve", expected, {20000801});
}

CASE("test_metkit_expand_7") {
    const char* text =
        "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param="
        "129,levelist=0.01/0.7";
    ExpectedOutput expected{{"class", {"rd"}},  {"expver", {"hl1m"}},       {"stream", {"oper"}}, {"time", {"0000"}},
                            {"domain", {"g"}},  {"type", {"fc"}},           {"levtype", {"pl"}},  {"step", {"24"}},
                            {"param", {"129"}}, {"levelist", {".01", ".7"}}};
    expand(text, "retrieve", expected, {20000801});
}

CASE("test_metkit_expand_8") {
    const char* text =
        "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param="
        "129,levelist=0.1/to/0.7/by/0.2";
    ExpectedOutput expected{{"class", {"rd"}},    {"expver", {"hl1m"}},
                            {"stream", {"oper"}}, {"time", {"0000"}},
                            {"domain", {"g"}},    {"type", {"fc"}},
                            {"levtype", {"pl"}},  {"step", {"24"}},
                            {"param", {"129"}},   {"levelist", {".1", ".3", ".5", ".7"}}};
    expand(text, "retrieve", expected, {20000801});
}

CASE("test_metkit_expand_9_strict") {
    const char* text = "retrieve,class=rd,expver=hm1u,stream=weeh,time=0000,date=20210101,domain=g,hdate=20190101";
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpansion expand(false, false);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        EXPECT_EQUAL(v.size(), 1);
    }
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpansion expand(false, true);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        EXPECT_EQUAL(v.size(), 1);
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,date=20240304,time=0,type=fc,levtype=sfc,levelist=1/2/"
            "3,step=0,param=2t,target=out";
        EXPECT_THROWS_AS(MarsRequest::parse(text, true), eckit::UserError);
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=FC,STREAM=OPER,EXPVER=0001,REPRES=GG,LEVTYPE=SFC,PARAM=167,DATE=20240304,TIME=0000,"
            "STEP=0,DOMAIN=G,TARGET=out";
        expand(text, expected, false);
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=enfo,date=20240304,time=0,type=cf,levtype=sfc,levelist=1/2/"
            "3,step=0,param=2t,number=1/2/3,target=out";
        EXPECT_THROWS_AS(MarsRequest::parse(text, true), eckit::UserError);
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=CF,STREAM=ENFO,EXPVER=0001,REPRES=SH,LEVTYPE=SFC,PARAM=167,DATE=20240304,TIME=0000,"
            "STEP=0,DOMAIN=G,TARGET=out";
        expand(text, expected, false);
    }
}

CASE("test_metkit_expand_10_strict") {
    const char* text = "retrieve,class=rd,expver=hm1u,stream=wees,time=0000,date=20210101,domain=g,hdate=20190101";
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpansion expand(false, false);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        EXPECT_EQUAL(v.size(), 1);
    }
}

CASE("test_metkit_expand_multirequest-1") {
    const std::string text = "ret,date=-5/to/-2.\nret,date=-1";
    std::istringstream in(text);
    std::vector<MarsRequest> reqs = MarsRequest::parse(in, true);
    EXPECT_EQUAL(reqs.size(), 2);
    ExpectedOutput expected{{"class", {"od"}},    {"domain", {"g"}},
                            {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                            {"levtype", {"pl"}},  {"param", {"129"}},
                            {"step", {"0"}},      {"stream", {"oper"}},
                            {"time", {"1200"}},   {"type", {"an"}}};
    expand(reqs.at(0), "retrieve", expected, {-5, -4, -3, -2});
    expand(reqs.at(1), "retrieve", expected, {-1});
}

void expandKeyThrows(const std::string& key, std::vector<std::string> values) {
    DummyContext ctx;
    static metkit::mars::MarsLanguage language("retrieve");
    metkit::mars::Type* t = language.type(key);
    EXPECT_THROWS_AS(t->expand(ctx, values), eckit::BadValue);
}
void expandKey(const std::string& key, std::vector<std::string> values, std::vector<std::string> expected) {
    DummyContext ctx;
    static metkit::mars::MarsLanguage language("retrieve");
    metkit::mars::Type* t = language.type(key);
    t->expand(ctx, values);
    EXPECT_EQUAL(expected, values);
}

void quantileThrows(std::vector<std::string> values) {
    expandKeyThrows("quantile", values);
}
void quantile(std::vector<std::string> values, std::vector<std::string> expected) {
    expandKey("quantile", values, expected);
}

CASE("test_metkit_expand_11_quantile") {
    quantileThrows({"-1:5"});
    quantileThrows({"0:-5"});
    quantileThrows({"6:5"});
    quantileThrows({"0:12"});
    quantile({"2:5"}, {"2:5"});
    quantile({"0:2", "1:2", "2:2"}, {"0:2", "1:2", "2:2"});
    quantile({"0:2", "1:3", "2:5"}, {"0:2", "1:3", "2:5"});

    quantileThrows({"to", "5:10"});
    quantileThrows({"3:5", "to"});
    quantileThrows({"3:5", "to", "5:10"});
    quantileThrows({"1:5", "to", "3:5", "by"});

    quantile({"0:5", "to", "0:5"}, {"0:5"});
    quantile({"3:3", "to", "3:3"}, {"3:3"});
    quantile({"0:5", "to", "5:5"}, {"0:5", "1:5", "2:5", "3:5", "4:5", "5:5"});
    quantile({"0:5", "to", "5:5", "by", "1"}, {"0:5", "1:5", "2:5", "3:5", "4:5", "5:5"});
    quantile({"0:5", "to", "5:5", "by", "2"}, {"0:5", "2:5", "4:5"});
    quantile({"0:5", "to", "5:5", "by", "3"}, {"0:5", "3:5"});
    quantile({"0:5", "to", "5:5", "by", "5"}, {"0:5", "5:5"});
    quantile({"0:5", "to", "5:5", "by", "6"}, {"0:5"});
    quantile({"2:5", "to", "5:5", "by", "2"}, {"2:5", "4:5"});
    quantile({"3:5", "to", "5:5", "by", "2"}, {"3:5", "5:5"});
    quantile({"4:5", "to", "5:5", "by", "2"}, {"4:5"});
    quantile({"0:10", "3:10", "to", "7:10", "by", "2", "10:10"}, {"0:10", "3:10", "5:10", "7:10", "10:10"});
    quantile({"3:5", "to", "2:5"}, {"3:5", "2:5"});
}


void timeThrows(std::vector<std::string> values) {
    expandKeyThrows("time", values);
}
void time(std::vector<std::string> values, std::vector<std::string> expected) {
    expandKey("time", values, expected);
}

CASE("test_metkit_expand_12_time") {
    timeThrows({"87"});
    timeThrows({"000012"});
    timeThrows({"0:0:12"});
    timeThrows({"12s"});
    time({"0"}, {"0000"});
    time({"0", "1", "6", "12", "18"}, {"0000", "0100", "0600", "1200", "1800"});
    time({"0", "1", "12"}, {"0000", "0100", "1200"});
    time({"0030"}, {"0030"});
    time({"30m"}, {"0030"});
    time({"00:30", "1:30", "02:50"}, {"0030", "0130", "0250"});
    time({"0h", "3h", "120m", "170m"}, {"0000", "0300", "0200", "0250"});

    timeThrows({"to", "5"});
    timeThrows({"3", "to"});
    timeThrows({"1", "to", "3", "by"});

    time({"0", "to", "0"}, {"0000"});
    time({"12", "to", "12"}, {"1200"});
    time({"0", "to", "12"}, {"0000", "0600", "1200"});
    time({"3", "to", "2"}, {"0300"});
    time({"12", "to", "4"}, {"1200", "0600"});
    time({"0", "to", "6", "by", "1"}, {"0000", "0100", "0200", "0300", "0400", "0500", "0600"});
    time({"0", "to", "6", "by", "2"}, {"0000", "0200", "0400", "0600"});
    time({"0", "to", "6", "by", "3"}, {"0000", "0300", "0600"});
    time({"0", "to", "6", "by", "4"}, {"0000", "0400"});
    time({"0", "to", "6", "by", "5"}, {"0000", "0500"});
    time({"0", "to", "6", "by", "6"}, {"0000", "0600"});
    time({"6", "to", "18"}, {"0600", "1200", "1800"});
    time({"1", "to", "6", "by", "1"}, {"0100", "0200", "0300", "0400", "0500", "0600"});
    time({"1", "to", "6", "by", "2"}, {"0100", "0300", "0500"});
    time({"1", "to", "6", "by", "3"}, {"0100", "0400"});
    time({"1", "to", "6", "by", "4"}, {"0100", "0500"});
    time({"1", "to", "6", "by", "5"}, {"0100", "0600"});
    time({"1", "to", "6", "by", "6"}, {"0100"});

    time({"1", "to", "3h", "by", "30m"}, {"0100", "0130", "0200", "0230", "0300"});
}


void stepThrows(std::vector<std::string> values) {
    expandKeyThrows("step", values);
}
void step(std::string valuesStr, std::string expectedStr) {
    eckit::Tokenizer parse("/");
    std::vector<std::string> values;
    std::vector<std::string> expected;

    parse(valuesStr, values);
    parse(expectedStr, expected);

    expandKey("step", values, expected);
}

CASE("test_metkit_expand_13_step") {
    step("0", "0");
    step("1", "1");
    step("24", "24");
    step("144", "144");
    step("012", "12");
    step("12:30", "12h30m");
    step("1:00", "1");
    step("1:0:0", "1");
    step("1h", "1");
    step("60m", "1");
    step("1h60m", "2");
    step("0:5", "5m");
    step("0:05", "5m");
    step("0:05:0", "5m");
    step("0:06", "6m");
    step("0:10", "10m");
    step("0:12", "12m");
    step("0:15", "15m");
    step("0:20", "20m");
    step("0:25", "25m");
    step("0:30", "30m");
    step("0:35", "35m");
    step("0:40", "40m");
    step("0:45", "45m");
    step("0:50", "50m");
    step("0:55", "55m");
    step("0-24", "0-24");
    step("0-24s", "0-24s");
    step("0-120s", "0-2m");
    step("0s-120m", "0-2");
    step("1-2", "1-2");
    step("30m-1", "30m-1");
    step("30m-90m", "30m-1h30m");

    step("0/to/24/by/3", "0/3/6/9/12/15/18/21/24");
    step("12/to/48/by/12", "12/24/36/48");
    step("12/to/47/by/12", "12/24/36");
    step("0/to/6/by/30m", "0/30m/1/1h30m/2/2h30m/3/3h30m/4/4h30m/5/5h30m/6");
    step("0-6/to/18-24/by/6", "0-6/6-12/12-18/18-24");
    step("0-24/to/48-72/by/24", "0-24/24-48/48-72");
    step("0/to/24/by/3/0-6/to/18-24/by/6", "0/3/6/9/12/15/18/21/24/0-6/6-12/12-18/18-24");
    step("0-24/to/24-48/by/90m",
         "0-24/1h30m-25h30m/3-27/4h30m-28h30m/6-30/7h30m-31h30m/9-33/10h30m-34h30m/12-36/13h30m-37h30m/15-39/"
         "16h30m-40h30m/18-42/19h30m-43h30m/21-45/22h30m-46h30m/24-48");
}


void activity(std::vector<std::string> values, std::vector<std::string> expected) {
    expandKey("activity", values, expected);
}
void experiment(std::vector<std::string> values, std::vector<std::string> expected) {
    expandKey("experiment", values, expected);
}
void model(std::vector<std::string> values, std::vector<std::string> expected) {
    expandKey("model", values, expected);
}

CASE("test_metkit_expand_lowercase") {
    activity({"ScenarioMIP"}, {"scenariomip"});
    activity({"CMIP6"}, {"cmip6"});
    activity({"ScenarioMIP"}, {"scenariomip"});
    activity({"cmip6"}, {"cmip6"});
    experiment({"SSP3-7.0"}, {"ssp3-7.0"});
    experiment({"ssp3-7.0"}, {"ssp3-7.0"});
    experiment({"hist"}, {"hist"});
    model({"IFS-NEMO"}, {"ifs-nemo"});
    model({"IFS"}, {"ifs"});
    model({"ifs-nemo"}, {"ifs-nemo"});
    model({"ifs"}, {"ifs"});
    model({"ICON"}, {"icon"});
    model({"icon"}, {"icon"});
}

CASE("test_metkit_expand_param") {
    {
        const char* text =
            "retrieve,class=od,expver=0079,stream=enfo,date=-1,time=00/12,type=pf,levtype=sfc,step=24,number=1/to/"
            "2,param=mucin/mucape/tprate";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 3);

        EXPECT_EQUAL(params[0], "228236");
        EXPECT_EQUAL(params[1], "228235");
        EXPECT_EQUAL(params[2], "260048");
    }
    {
        const char* text =
            "retrieve,class=od,expver=0079,stream=enfh,date=-1,time=00/12,type=fcmean,levtype=sfc,step=24,number=1/to/"
            "2,param=mucin/mucape/tprate";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 3);

        EXPECT_EQUAL(params[0], "228236");
        EXPECT_EQUAL(params[1], "228235");
        EXPECT_EQUAL(params[2], "172228");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=wave,date=-1,time=00/12,type=an,levtype=sfc,step=24,param=2dfd ";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140251");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=enwh,date=-1,time=00/12,type=cf,levtype=sfc,step=24,param=tmax";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140217");
    }
    {
        const char* text =
            "retrieve,class=ai,expver=1,stream=oper,model=aifs-singlw,date=-1,time=00/"
            "12,type=pf,levtype=pl,step=24,param=t";
        EXPECT_THROWS_AS(MarsRequest::parse(text, true), eckit::UserError);
    }
    {
        const char* text =
            "retrieve,class=ai,expver=1,stream=oper,model=aifs-single,date=-1,time=00/"
            "12,type=pf,levtype=pl,step=24,param=t";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "130");
    }
    {
        const char* text =
            "retrieve,class=od,date=20240723,domain=g,expver=0079,levtype=sfc,param=asn/cp/lsp/sf/tcc/"
            "tp,step=0,stream=oper,time=0000,type=fc";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 6);

        EXPECT_EQUAL(params[0], "32");
        EXPECT_EQUAL(params[1], "143");
        EXPECT_EQUAL(params[2], "142");
        EXPECT_EQUAL(params[3], "144");
        EXPECT_EQUAL(params[4], "164");
        EXPECT_EQUAL(params[5], "228");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=msmm,date=-1,time=0000,type=em,levtype=sfc,step=24,param=e";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "172182");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=msmm,date=-1,time=0000,type=em,levtype=sfc,step=24,param=e/erate";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 2);

        EXPECT_EQUAL(params[0], "172182");
        EXPECT_EQUAL(params[1], "172182");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=enwh,date=-1,time=0000,type=pf,levtype=sfc,step=24,param=sh10";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140120");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=enwh,date=-1,time=0000,type=pf,levtype=sfc,step=24,param=p1ww";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140223");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=waef,date=-1,time=0000,type=cf,levtype=sfc,step=24,param=WSK/MWP";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 2);

        EXPECT_EQUAL(params[0], "140252");
        EXPECT_EQUAL(params[1], "140232");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=eefo,date=-1,time=0000,type=fcmean,levtype=sfc,step=24,param=MSL";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "151");
    }
    {
        const char* text =
            "retrieve,class=od,expver=1,stream=eefo,date=-1,time=0000,type=fcmean,levtype=sfc,step=24,param=strda";
        MarsRequest r = MarsRequest::parse(text);
        auto params   = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "171175");
    }
}

CASE("test_metkit_expand_d1") {
    {
        const char* text = "retrieve,class=d1,dataset=extremes-dt,date=-1";
        ExpectedOutput expected{{"class", {"d1"}},    {"dataset", {"extremes-dt"}},
                                {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                {"levtype", {"pl"}},  {"param", {"129"}},
                                {"step", {"0"}},      {"stream", {"oper"}},
                                {"time", {"1200"}},   {"type", {"an"}}};
        expand(text, "retrieve", expected, {-1});
    }
    {
        const char* text = "retrieve,class=d1,dataset=extreme-dt,date=-1";
        ExpectedOutput expected{{"class", {"d1"}},    {"dataset", {"extremes-dt"}},
                                {"expver", {"0001"}}, {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                {"levtype", {"pl"}},  {"param", {"129"}},
                                {"step", {"0"}},      {"stream", {"oper"}},
                                {"time", {"1200"}},   {"type", {"an"}}};
        expand(text, "retrieve", expected, {-1});
    }
    {
        const char* text =
            "retrieve,class=d1,dataset=climate-dt,levtype=pl,date=20000101,activity=CMIP6,experiment=hist,model=IFS-"
            "NEMO,generation=1,realization=1,resolution=high,stream=clte,type=fc,param=134/137";
        ExpectedOutput expected{{"class", {"d1"}},        {"dataset", {"climate-dt"}},
                                {"activity", {"cmip6"}},  {"experiment", {"hist"}},
                                {"model", {"ifs-nemo"}},  {"generation", {"1"}},
                                {"realization", {"1"}},   {"resolution", {"high"}},
                                {"expver", {"0001"}},     {"time", {"1200"}},
                                {"stream", {"clte"}},     {"type", {"fc"}},
                                {"levtype", {"pl"}},      {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                {"param", {"134", "137"}}};
        expand(text, "retrieve", expected, {20000101});
    }
    {
        const char* text =
            "retrieve,date=20120515,time=0000,dataset=climate-dt,activity=cmip6,experiment=hist,generation=1,model="
            "icon,realization=1,georef=acbdef,resolution=high,class=d1,expver=0001,type=fc,stream=clte,levelist=1,"
            "levtype=o3d,param=263500";
        const char* text2 =
            "retrieve,date=20120515,time=0000,dataset=climate-dt,activity=cmip6,experiment=hist,generation=1,model="
            "icon,realization=1,resolution=high,class=d1,expver=0001,type=fc,stream=clte,levelist=1,"
            "levtype=o3d,param=263500";
        ExpectedOutput expected{{"class", {"d1"}},        {"dataset", {"climate-dt"}}, {"activity", {"cmip6"}},
                                {"experiment", {"hist"}}, {"model", {"icon"}},         {"generation", {"1"}},
                                {"realization", {"1"}},   {"resolution", {"high"}},    {"expver", {"0001"}},
                                {"time", {"0000"}},       {"stream", {"clte"}},        {"type", {"fc"}},
                                {"levtype", {"o3d"}},     {"levelist", {"1"}},         {"param", {"263500"}}};
        expand(text2, "retrieve", expected, {20120515});
    }
}
CASE("test_metkit_expand_ng") {
    {
        const char* text =
            "retrieve,class=ng,date=20000101,activity=CMIP6,experiment=hist,model=IFS-NEMO,generation=1,realization=1,"
            "resolution=high,stream=clte,type=fc,levtype=pl,param=134/137";
        ExpectedOutput expected{{"class", {"ng"}},
                                {"levtype", {"pl"}},
                                {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                {"activity", {"cmip6"}},
                                {"experiment", {"hist"}},
                                {"model", {"ifs-nemo"}},
                                {"generation", {"1"}},
                                {"realization", {"1"}},
                                {"resolution", {"high"}},
                                {"expver", {"0001"}},
                                {"date", {"20000101"}},
                                {"time", {"1200"}},
                                {"stream", {"clte"}},
                                {"type", {"fc"}},
                                {"param", {"134", "137"}}};
        expand(text, "retrieve", expected, {20000101});
    }
}
CASE("test_metkit_expand_ai") {
    {
        const char* text =
            "retrieve,class=ai,date=20250208,time=1800,expver=9999,model=aifs-single,type=fc,levtype=sfc,param=169";
        ExpectedOutput expected{{"class", {"ai"}},    {"domain", {"g"}}, {"expver", {"9999"}},
                                {"levtype", {"sfc"}}, {"step", {"0"}},   {"stream", {"oper"}},
                                {"time", {"1800"}},   {"type", {"fc"}},  {"model", {"aifs-single"}},
                                {"param", {"169"}}};
        expand(text, "retrieve", expected, {20250208});
    }
}

CASE("test_metkit_expand_list") {
    {
        const char* text =
            "list,date=20250105,domain=g,levtype=pl,expver="
            "0001,step=0,stream=oper,levelist=1000/850/700/500/400/"
            "300,time=1200,type=an,param=129";
        ExpectedOutput expected{{"class", {"od"}},
                                {"date", {"20250105"}},
                                {"domain", {"g"}},
                                {"levtype", {"pl"}},
                                {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                {"expver", {"0001"}},
                                {"time", {"1200"}},
                                {"stream", {"oper"}},
                                {"type", {"an"}},
                                {"param", {"129"}}};
        expand(text, "list", expected, {20250105});
    }
    {
        const char* text = "list,class=tr,date=20250105";
        ExpectedOutput expected{{"class", {"tr"}}, {"date", {"20250105"}}};
        expand(text, "list", expected, {20250105});
    }
}

CASE("test_metkit_expand_read") {
    {
        const char* text =
            "read,class=tr,date=20250105,domain=g,levtype=pl,expver=0001,step=0,stream=oper,"
            "levelist=1000/850/700/500/400/300,time=1200,type=an,param=129";
        ExpectedOutput expected{{"class", {"tr"}},
                                {"date", {"20250105"}},
                                {"domain", {"g"}},
                                {"levtype", {"pl"}},
                                {"levelist", {"1000", "850", "700", "500", "400", "300"}},
                                {"expver", {"0001"}},
                                {"time", {"1200"}},
                                {"stream", {"oper"}},
                                {"type", {"an"}},
                                {"param", {"129"}}};
        expand(text, "read", expected, {20250105});
    }
    {
        const char* text = "read,date=20250105,param=129";
        ExpectedOutput expected{{"date", {"20250105"}}, {"param", {"129"}}};
        expand(text, "read", expected, {20250105});
    }
}

CASE("test_metkit_expand_clmn") {
    {
        const char* text =
            "retrieve,class=d1,expver=1,dataset=climate-dt,activity=story-nudging,experiment=Tplus2.0K,generation=1,"
            "model=IFS-FESOM,realization=1,stream=clmn,year=2024,month=october,resolution=standard,type=fc,levtype=sfc,"
            "param=144";
        ExpectedOutput expected{{"class", {"d1"}},
                                {"dataset", {"climate-dt"}},
                                {"activity", {"story-nudging"}},
                                {"experiment", {"tplus2.0k"}},
                                {"generation", {"1"}},
                                {"model", {"ifs-fesom"}},
                                {"realization", {"1"}},
                                {"expver", {"0001"}},
                                {"stream", {"clmn"}},
                                {"year", {"2024"}},
                                {"month", {"10"}},
                                {"resolution", {"standard"}},
                                {"type", {"fc"}},
                                {"levtype", {"sfc"}},
                                {"param", {"144"}}};
        expand(text, "retrieve", expected, {});
    }
}

CASE("test_metkit_expand_frequency") {
    const char* text =
        "retrieve,class=od,date=20250401,direction=31,domain=g,expver=0001,frequency=7,levtype=sfc,param=140217,step=0,"
        "stream=wave,time=1200,type=an";
    const char* expected =
        "RETRIEVE,CLASS=OD,TYPE=AN,STREAM=WAVE,EXPVER=0001,REPRES=SH,LEVTYPE=SFC,PARAM=140217,DATE=20250401,TIME=1200,"
        "STEP=0,DOMAIN=G,FREQUENCY=7,DIRECTION=31";
    expand(text, expected);
}

CASE("test_metkit_expand_") {
    // issues from https://confluence.ecmwf.int/pages/viewpage.action?pageId=496866851
    {
        // https://jira.ecmwf.int/browse/MARSC-218
        const char* text =
            "retrieve,accuracy=10,class=ea,date=1969-03-28,expver=11,grid=0.25/"
            "0.25,levtype=sfc,packing=si,param=142.128/143.128/151.128/165.128/166.128,step=0/6/12/18/24/30/36/42/48/"
            "54/60/66/72/78/84/90/96/102/108/114/120/132/144/156/168/180/192/204/216/228/"
            "240,stream=oper,time=00:00:00,type=fc,target=\"reference.rFP7XB.data\"";
        const char* expected =
            "RETRIEVE,CLASS=EA,TYPE=FC,STREAM=OPER,EXPVER=0011,LEVTYPE=SFC,PARAM=142/143/151/165/"
            "166,DATE=19690328,TIME=0000,STEP=0/6/12/18/24/30/36/42/48/54/60/66/72/78/84/90/96/102/108/114/120/132/144/"
            "156/168/180/192/204/216/228/240,DOMAIN=G,TARGET=reference.rFP7XB.data,RESOL=AUTO,ACCURACY=10,GRID=.25/"
            ".25,PACKING=SIMPLE";
        expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-221
        const char* text =
            "retrieve,accuracy=12,area=90.0/0.0/-90.0/359.5,date=20240102,domain=g,grid=0.5/"
            "0.5,leve=off,levtype=sfc,padding=0,param=134/137/165/166/167/168/"
            "235,stream=da,style=dissemination,time=00,type=an,target=\"reference.tzpUX7.data\"";
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=AN,STREAM=OPER,EXPVER=0001,REPRES=GG,LEVTYPE=SFC,PARAM=134/137/165/166/167/168/"
            "235,DATE=20240102,TIME=0000,STEP=0,DOMAIN=G,TARGET=reference.tzpUX7.data,RESOL=AV,ACCURACY=12,STYLE="
            "DISSEMINATION,AREA=90/0/-90/359.5,GRID=.5/.5,PADDING=0";
        expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-212
        const char* text =
            "retrieve,accuracy=16,area=14.8/-19.6/-14.5/19.8,class=od,date=20230810,expver=1,grid=0.09/0.09,levelist=1/"
            "to/137,levtype=ml,number=-1,param=z,process=local,rotation=-78.8/"
            "-61.0,step=000,stream=scda,time=18,type=an,target=\"reference.ect1qF.data\"";
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=AN,STREAM=SCDA,EXPVER=0001,REPRES=SH,LEVTYPE=ML,LEVELIST=1/2/3/4/5/6/7/8/9/10/11/"
            "12/13/14/15/16/17/18/19/20/21/22/23/24/25/26/27/28/29/30/31/32/33/34/35/36/37/38/39/40/41/42/43/44/45/46/"
            "47/48/49/50/51/52/53/54/55/56/57/58/59/60/61/62/63/64/65/66/67/68/69/70/71/72/73/74/75/76/77/78/79/80/81/"
            "82/83/84/85/86/87/88/89/90/91/92/93/94/95/96/97/98/99/100/101/102/103/104/105/106/107/108/109/110/111/112/"
            "113/114/115/116/117/118/119/120/121/122/123/124/125/126/127/128/129/130/131/132/133/134/135/136/"
            "137,PARAM=129,DATE=20230810,TIME=1800,STEP=0,DOMAIN=G,TARGET=reference.ect1qF.data,RESOL=AUTO,ACCURACY=16,"
            "AREA=14.8/-19.6/-14.5/19.8,ROTATION=-78.8/-61,GRID=.09/.09,PROCESS=LOCAL";
        expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-210
        const char* text =
            "retrieve,accuracy=24,area=90.0/-179.0/-90.0/180.0,class=od,dataset=none,date=20231231/to/"
            "20231231,expver=1,grid=off,levelist=1,levtype=ml,number=off,padding=0,param=152.128,resol=255,step=00,"
            "stream=oper,time=00/12,type=an,target=\"reference.YYhupw.data\"";
        /// @todo DATASET
        const char* expected =
            "RETRIEVE,DATASET=none,CLASS=OD,TYPE=AN,STREAM=OPER,EXPVER=0001,REPRES=SH,LEVTYPE=ML,LEVELIST=1,PARAM=152."
            "128,DATE=20231231,TIME=0000/"
            "1200,STEP=0,DOMAIN=G,TARGET=reference.YYhupw.data,RESOL=255,ACCURACY=24,AREA=90/-179/-90/180,PADDING=0";
        // expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-220
        const char* text =
            "retrieve,an_offset=9,class=rd,date=20210828,expver=i8k5,gaussian=regular,grid=4000,levtype=sfc,param=151."
            "128/165.128/166.128,step=15,stream=da,time=00,type=fc,target=\"reference.6Zr8N7.data\"";
        const char* expected =
            "RETRIEVE,CLASS=RD,TYPE=FC,STREAM=OPER,EXPVER=i8k5,REPRES=SH,LEVTYPE=SFC,PARAM=151/165/"
            "166,DATE=20210828,TIME=0000,STEP=15,DOMAIN=G,TARGET=reference.6Zr8N7.data,RESOL=AUTO,GRID=F4000";
        expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-214
        const char* text =
            "retrieve,anoffset=90,class=ce,database=marser,date=20240102,domain=g,expver=0001,level=3,levtype=sol,"
            "model=lisflood,origin=ecmf,param=260199,step=6/to/72/by/"
            "6,stream=efas,time=0000,type=sfo,expect=any,target=\"reference.43PsBL.data\"";
        const char* expected =
            "RETRIEVE,CLASS=CE,TYPE=SFO,STREAM=EFAS,EXPVER=0001,MODEL=lisflood,REPRES=SH,LEVTYPE=SOL,LEVELIST=3,PARAM="
            "260199,DATE=20240102,TIME=0000,STEP=6/12/18/24/30/36/42/48/54/60/66/"
            "72,ANOFFSET=90,DOMAIN=G,ORIGIN=ECMF,EXPECT=ANY,TARGET=reference.43PsBL.data,DATABASE=marser";
        expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-222
        const char* text =
            "retrieve,class=rd,date=20201204,expver=hk3a,obsgroup=22,reportype=21001,stream=lwda,time=1200,type=mfb,"
            "target=\"reference.vYyJf6.data\"";
        /// @todo DUPLICATES
        const char* expected =
            "RETRIEVE,CLASS=RD,TYPE=MFB,STREAM=LWDA,EXPVER=hk3a,REPRES=BU,OBSGROUP=AMSUA_AS,REPORTYPE=21001,obstype=1,"
            "DATE=20201204,TIME=1200,DOMAIN=G,TARGET=reference.vYyJf6.data,DUPLICATES=KEEP";
        // expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-219
        const char* text =
            "retrieve,class=od,date=20231205,expver=0001,obstype=gpsro,stream=lwda,time=18,type=ai,target=\"reference."
            "E2RRc8.data\"";
        /// @todo OBSTYPE, OBSGROUP
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=AI,STREAM=LWDA,EXPVER=0001,REPRES=BU,OBSTYPE=250,DATE=20231205,TIME=1800,DOMAIN=G,"
            "TARGET=reference.E2RRc8.data,DUPLICATES=KEEP";
        // expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-213
        const char* text =
            "retrieve,class=od,date=20230821,expect=any,expver=0001,levtype=sfc,param=70.228/71.228/72.228/73.228/"
            "74.228,stream=da,time=12,type=gai,target=\"reference.1e7AY1.data\"";
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=GAI,STREAM=OPER,EXPVER=0001,REPRES=GG,LEVTYPE=SFC,PARAM=228070/228071/228072/"
            "228073/228074,DATE=20230821,TIME=1200,STEP=0,DOMAIN=G,TARGET=reference.1e7AY1.data,EXPECT=ANY";
        expand(text, expected);
    }
    {
        const char* text =
            "retrieve,accuracy=16,area=14.8/-19.6/-14.5/19.8,class=od,date=20230810,expver=1,grid=0.09/0.09,levelist=1/"
            "to/137,levtype=ml,number=-1,param=z,process=local,rotation=-78.8/"
            "-61.0,step=000,stream=scda,time=18,type=an,target=\"reference.ect1qF.data\"";
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=AN,STREAM=SCDA,EXPVER=0001,REPRES=SH,LEVTYPE=ML,LEVELIST=1/2/3/4/5/6/7/8/9/10/11/"
            "12/13/14/15/16/17/18/19/20/21/22/23/24/25/26/27/28/29/30/31/32/33/34/35/36/37/38/39/40/41/42/43/44/45/46/"
            "47/48/49/50/51/52/53/54/55/56/57/58/59/60/61/62/63/64/65/66/67/68/69/70/71/72/73/74/75/76/77/78/79/80/81/"
            "82/83/84/85/86/87/88/89/90/91/92/93/94/95/96/97/98/99/100/101/102/103/104/105/106/107/108/109/110/111/112/"
            "113/114/115/116/117/118/119/120/121/122/123/124/125/126/127/128/129/130/131/132/133/134/135/136/"
            "137,PARAM=129,DATE=20230810,TIME=1800,STEP=0,DOMAIN=G,TARGET=reference.ect1qF.data,RESOL=AUTO,ACCURACY=16,"
            "AREA=14.8/-19.6/-14.5/19.8,ROTATION=-78.8/-61,GRID=.09/.09,PROCESS=LOCAL";
        expand(text, expected);
    }
    {
        const char* text =
            "retrieve,accuracy=16,area=60.0/-60.0/-60.0/60.0,class=ea,date=20101029,expver=1,grid=0.30/0.30,levelist=1/"
            "to/137,levtype=ml,number=-1,param=q/t/u/v/lnsp/z,rotation=0.0/"
            "0.0,step=000,stream=oper,time=15:00:00,type=an,target=\"reference.1OEDK0.data\"";
        const char* expected =
            "RETRIEVE,CLASS=EA,TYPE=AN,STREAM=OPER,EXPVER=0001,LEVTYPE=ML,LEVELIST=1/2/3/4/5/6/7/8/9/10/11/12/13/14/15/"
            "16/17/18/19/20/21/22/23/24/25/26/27/28/29/30/31/32/33/34/35/36/37/38/39/40/41/42/43/44/45/46/47/48/49/50/"
            "51/52/53/54/55/56/57/58/59/60/61/62/63/64/65/66/67/68/69/70/71/72/73/74/75/76/77/78/79/80/81/82/83/84/85/"
            "86/87/88/89/90/91/92/93/94/95/96/97/98/99/100/101/102/103/104/105/106/107/108/109/110/111/112/113/114/115/"
            "116/117/118/119/120/121/122/123/124/125/126/127/128/129/130/131/132/133/134/135/136/137,PARAM=133/130/131/"
            "132/152/129,TIME=1500,STEP=0,DOMAIN=G,TARGET=reference.1OEDK0.data,RESOL=AUTO,ACCURACY=16,AREA=60/-60/-60/"
            "60,ROTATION=0/0,GRID=.3/.3,DATE=20101029";
        expand(text, expected);
    }
    {
        const char* text =
            "retrieve,accuracy=12,area=90.0/0.0/-90.0/359.5,date=20240102,domain=g,grid=0.5/"
            "0.5,leve=off,levtype=sfc,padding=0,param=134/137/165/166/167/168/"
            "235,stream=da,style=dissemination,time=00,type=an,target=\"reference.tzpUX7.data\"";
        const char* expected =
            "RETRIEVE,CLASS=OD,TYPE=AN,STREAM=OPER,EXPVER=0001,REPRES=GG,LEVTYPE=SFC,PARAM=134/137/165/166/167/168/"
            "235,DATE=20240102,TIME=0000,STEP=0,DOMAIN=G,TARGET=reference.tzpUX7.data,RESOL=AV,ACCURACY=12,STYLE="
            "DISSEMINATION,AREA=90/0/-90/359.5,GRID=.5/.5,PADDING=0";
        expand(text, expected);
    }
    {
        // https://jira.ecmwf.int/browse/MARSC-211
        const char* text =
            "retrieve,class=ce,database=marser,date=20240102,domain=g,expver=0001,level=2,levtype=sol,model=lisflood,"
            "origin=ecmf,param=260199,step=6/to/240/by/"
            "6,stream=efas,time=0000,type=fc,target=\"reference.JkqWoW.data\"";
        const char* expected =
            "RETRIEVE,CLASS=CE,TYPE=FC,STREAM=EFAS,EXPVER=0001,MODEL=lisflood,REPRES=SH,LEVTYPE=SOL,LEVELIST=2,PARAM="
            "260199,DATE=20240102,TIME=0000,STEP=6/12/18/24/30/36/42/48/54/60/66/72/78/84/90/96/102/108/114/120/126/"
            "132/138/144/150/156/162/168/174/180/186/192/198/204/210/216/222/228/234/"
            "240,DOMAIN=G,ORIGIN=ECMF,TARGET=reference.JkqWoW.data";
        expand(text, expected);
    }
}

//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
