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
#include "metkit/mars/MarsExpension.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/Type.h"

#include "eckit/testing/Test.h"
#include "eckit/utils/Tokenizer.h"

using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

CASE( "test_metkit_expand_1" ) {
    const char* text = "ret,date=-5/to/-1";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}


CASE( "test_metkit_expand_2" ) {
    const char* text = "ret";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);

    const std::vector< std::string >& dates = r.values("date");
    EXPECT(dates.size() == 1);

    eckit::Date today(0);
    std::ostringstream oss;

    oss << today.yyyymmdd();
    EXPECT(dates[0] == oss.str());


}

CASE( "test_metkit_expand_3" ) {
    const char* text = "ret,date=-5/to/-1,grid=n640";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}


CASE( "test_metkit_expand_4" ) {
    const char* text = "ret,date=-5/to/-1,grid=o640";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_5" ) {
    const char* text = "retrieve,class=od,date=20050601,diagnostic=1,expver=1,iteration=0,levelist=1,levtype=ml,param=155.129,stream=sens,time=1200,type=sg";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_6" ) {
    const char* text = "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param=129,levelist=1/to/31";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_7" ) {
    const char* text = "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param=129,levelist=0.01/0.7";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_8" ) {
    const char* text = "retrieve,class=rd,expver=hl1m,stream=oper,date=20000801,time=0000,domain=g,type=fc,levtype=pl,step=24,param=129,levelist=0.1/to/0.7/by/0.2";
    MarsRequest r = MarsRequest::parse(text);
    r.dump(std::cout);
}

CASE( "test_metkit_expand_9_strict" ) {
    const char* text = "retrieve,class=rd,expver=hm1u,stream=weeh,time=0000,date=20210101,domain=g,hdate=20190101";
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, false);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        ASSERT(v.size() == 1);
        v[0].dump(std::cout);
    }
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, true);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        ASSERT(v.size() == 1);
        v[0].dump(std::cout);
    }
}

CASE( "test_metkit_expand_10_strict" ) {
    const char* text = "retrieve,class=rd,expver=hm1u,stream=wees,time=0000,date=20210101,domain=g,hdate=20190101";
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, false);
        std::vector<MarsRequest> v = expand.expand(parser.parse());

        ASSERT(v.size() == 1);
        v[0].dump(std::cout);
    }
    {
        std::istringstream in(text);
        MarsParser parser(in);
        MarsExpension expand(false, true);
        EXPECT_THROWS(expand.expand(parser.parse()));
    }
}

void expandKeyThrows(const std::string& key, std::vector<std::string> values) {
    DummyContext ctx;
    static metkit::mars::MarsLanguage language("retrieve");
    metkit::mars::Type* t = language.type(key);
    std::cout << key << "Throws " << values << std::endl;
    EXPECT_THROWS(t->expand(ctx, values));
}
void expandKey(const std::string& key, std::vector<std::string> values, std::vector<std::string> expected) {
    DummyContext ctx;
    static metkit::mars::MarsLanguage language("retrieve");
    metkit::mars::Type* t = language.type(key);
    std::cout << key << " " << values;
    t->expand(ctx, values);
    std::cout << " ==> " << values << " - expected " << expected << std::endl;
    ASSERT(values == expected);
}

void quantileThrows(std::vector<std::string> values) {
    expandKeyThrows("quantile", values);
}
void quantile(std::vector<std::string> values, std::vector<std::string> expected) {
    expandKey("quantile", values, expected);
}

CASE( "test_metkit_expand_11_quantile" ) {
    quantileThrows({"-1:5"});
    quantileThrows({"0:-5"});
    quantileThrows({"6:5"});
    quantileThrows({"0:12"});
    quantile({"2:5"}, {"2:5"});
    quantile({"0:2","1:2","2:2"}, {"0:2","1:2","2:2"});
    quantile({"0:2","1:3","2:5"}, {"0:2","1:3","2:5"});

    quantileThrows({"to","5:10"});
    quantileThrows({"3:5","to"});
    quantileThrows({"3:5","to","5:10"});
    quantileThrows({"3:5","to","2:5"});
    quantileThrows({"1:5","to","3:5","by"});
    quantileThrows({"1:5","to","3:5","by","1:5"});

    quantile({"0:5","to","0:5"}, {"0:5"});
    quantile({"3:3","to","3:3"}, {"3:3"});
    quantile({"0:5","to","5:5"}, {"0:5","1:5","2:5","3:5","4:5","5:5"});
    quantile({"0:5","to","5:5","by","1"}, {"0:5","1:5","2:5","3:5","4:5","5:5"});
    quantile({"0:5","to","5:5","by","2"}, {"0:5","2:5","4:5"});
    quantile({"0:5","to","5:5","by","3"}, {"0:5","3:5"});
    quantile({"0:5","to","5:5","by","5"}, {"0:5","5:5"});
    quantile({"0:5","to","5:5","by","6"}, {"0:5"});
    quantile({"2:5","to","5:5","by","2"}, {"2:5","4:5"});
    quantile({"3:5","to","5:5","by","2"}, {"3:5","5:5"});
    quantile({"4:5","to","5:5","by","2"}, {"4:5"});
    quantile({"0:10","3:10","to","7:10","by","2","10:10"}, {"0:10","3:10","5:10","7:10","10:10"});
}


void timeThrows(std::vector<std::string> values) {
    expandKeyThrows("time", values);
}
void time(std::vector<std::string> values, std::vector<std::string> expected) {
    expandKey("time", values, expected);
}

CASE( "test_metkit_expand_12_time" ) {
    timeThrows({"87"});
    timeThrows({"000012"});
    timeThrows({"0:0:12"});
    timeThrows({"12s"});
    time({"0"}, {"0000"});
    time({"0","1","6","12","18"}, {"0000","0100","0600","1200","1800"});
    time({"0","1","12"}, {"0000","0100","1200"});
    time({"0030"}, {"0030"});
    time({"30m"}, {"0030"});
    time({"00:30","1:30","02:50"}, {"0030","0130","0250"});
    time({"0h","3h","120m","170m"}, {"0000","0300","0200","0250"});

    timeThrows({"to","5"});
    timeThrows({"3","to"});
    timeThrows({"3","to","2"});
    timeThrows({"1","to","3","by"});

    time({"0","to","0"}, {"0000"});
    time({"12","to","12"}, {"1200"});
    time({"0","to","12"}, {"0000", "0600", "1200"});
    time({"0","to","6","by","1"}, {"0000", "0100", "0200", "0300", "0400", "0500", "0600"});
    time({"0","to","6","by","2"}, {"0000", "0200", "0400", "0600"});
    time({"0","to","6","by","3"}, {"0000", "0300", "0600"});
    time({"0","to","6","by","4"}, {"0000", "0400"});
    time({"0","to","6","by","5"}, {"0000", "0500"});
    time({"0","to","6","by","6"}, {"0000", "0600"});
    time({"6","to","18"}, {"0600", "1200", "1800"});
    time({"1","to","6","by","1"}, {"0100", "0200", "0300", "0400", "0500", "0600"});
    time({"1","to","6","by","2"}, {"0100", "0300", "0500"});
    time({"1","to","6","by","3"}, {"0100", "0400"});
    time({"1","to","6","by","4"}, {"0100", "0500"});
    time({"1","to","6","by","5"}, {"0100", "0600"});
    time({"1","to","6","by","6"}, {"0100"});

    time({"1","to","3h","by","30m"}, {"0100", "0130", "0200", "0230", "0300"});
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

CASE( "test_metkit_expand_13_step" ) {
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
    step("0-24/to/24-48/by/90m", "0-24/1h30m-25h30m/3-27/4h30m-28h30m/6-30/7h30m-31h30m/9-33/10h30m-34h30m/12-36/13h30m-37h30m/15-39/16h30m-40h30m/18-42/19h30m-43h30m/21-45/22h30m-46h30m/24-48");
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

CASE( "test_metkit_expand_lowercase" ) {
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

CASE( "test_metkit_expand_param" ) {
    {
        const char* text = "retrieve,class=od,expver=0079,stream=enfo,date=-1,time=00/12,type=pf,levtype=sfc,step=24,number=1/to/2,param=mucin/mucape/tprate";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 3);

        EXPECT_EQUAL(params[0], "228236");
        EXPECT_EQUAL(params[1], "228235");
        EXPECT_EQUAL(params[2], "260048");
    }
    {
        const char* text = "retrieve,class=od,expver=0079,stream=enfh,date=-1,time=00/12,type=fcmean,levtype=sfc,step=24,number=1/to/2,param=mucin/mucape/tprate";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 3);

        EXPECT_EQUAL(params[0], "228236");
        EXPECT_EQUAL(params[1], "228235");
        EXPECT_EQUAL(params[2], "172228");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=wave,date=-1,time=00/12,type=an,levtype=sfc,step=24,param=2dfd ";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140251");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=enwh,date=-1,time=00/12,type=cf,levtype=sfc,step=24,param=tmax";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140217");
    }
    {
        const char* text = "retrieve,class=ai,expver=1,stream=oper,date=-1,time=00/12,type=pf,levtype=pl,step=24,param=t";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "130");
    }
    {
        const char* text = "retrieve,class=od,date=20240723,domain=g,expver=0079,levtype=sfc,param=asn/cp/lsp/sf/tcc/tp,step=0,stream=oper,time=0000,type=fc";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 6);

        EXPECT_EQUAL(params[0], "32");
        EXPECT_EQUAL(params[1], "143");
        EXPECT_EQUAL(params[2], "142");
        EXPECT_EQUAL(params[3], "144");
        EXPECT_EQUAL(params[4], "164");
        EXPECT_EQUAL(params[5], "228");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=msmm,date=-1,time=0000,type=em,levtype=sfc,step=24,param=e";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "172182");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=msmm,date=-1,time=0000,type=em,levtype=sfc,step=24,param=e/erate";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 2);

        EXPECT_EQUAL(params[0], "172182");
        EXPECT_EQUAL(params[1], "172182");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=enwh,date=-1,time=0000,type=pf,levtype=sfc,step=24,param=sh10";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140120");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=enwh,date=-1,time=0000,type=pf,levtype=sfc,step=24,param=p1ww";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "140223");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=waef,date=-1,time=0000,type=cf,levtype=sfc,step=24,param=WSK/MWP";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 2);

        EXPECT_EQUAL(params[0], "140252");
        EXPECT_EQUAL(params[1], "140232");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=eefo,date=-1,time=0000,type=fcmean,levtype=sfc,step=24,param=MSL";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "151");
    }
    {
        const char* text = "retrieve,class=od,expver=1,stream=eefo,date=-1,time=0000,type=fcmean,levtype=sfc,step=24,param=strda";
        MarsRequest r = MarsRequest::parse(text);
        auto params = r.values("param");
        EXPECT_EQUAL(params.size(), 1);

        EXPECT_EQUAL(params[0], "171175");
    }
}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
