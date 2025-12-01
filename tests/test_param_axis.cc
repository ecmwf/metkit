/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_param_axis.cc
/// @author Baudouin Raoult
/// @date   Mai 2019

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Param.h"
#include "metkit/mars/ParamID.h"
#include "metkit/mars/Type.h"

namespace metkit::mars::test {

//-----------------------------------------------------------------------------
static std::ostream& operator<<(std::ostream& out, const std::vector<Param>& params) {
    out << '[';
    const char* sep = "";
    for (auto p : params) {
        out << sep << p;
        sep = ", ";
    }
    out << ']';
    return out;
}

static void test_param_axis(const std::vector<std::string>& user, const std::vector<std::string>& axis,
                            const std::vector<std::string>& expect, bool expectWind, bool fullTableDropping) {

    bool windRequested = false;

    MarsRequest ignore;

    std::vector<Param> params(user.begin(), user.end());
    std::vector<Param> expected(expect.begin(), expect.end());
    std::vector<Param> index(axis.begin(), axis.end());

    std::sort(index.begin(), index.end());

    std::cout << "Axis:" << index << std::endl;
    std::cout << "User:" << params << std::endl;
    std::cout << "Wind:" << false << std::endl;

    ParamID::normalise(ignore, params, index, windRequested, fullTableDropping);

    std::cout << "Expected Params:" << expected << std::endl;
    std::cout << "Returned Params:" << params << std::endl;
    std::cout << "Expected Wind:" << expectWind << std::endl;
    std::cout << "Returned Wind:" << windRequested << std::endl;

    EXPECT_EQUAL(expectWind, windRequested);
    EXPECT_EQUAL(params, expected);
}


static void test_param_axis(const std::vector<std::string>& user, const std::vector<std::string>& axis,
                            const std::vector<std::string>& expect, bool expectWind) {

    bool windRequested = false;

    MarsRequest ignore;

    std::vector<Param> params(user.begin(), user.end());
    std::vector<Param> expected(expect.begin(), expect.end());
    std::vector<Param> index(axis.begin(), axis.end());

    std::sort(index.begin(), index.end());


    std::cout << "Axis:" << index << std::endl;
    std::cout << "User:" << params << std::endl;
    std::cout << "Wind:" << false << std::endl;


    ParamID::normalise(ignore, params, index, windRequested);

    std::cout << "Expected Params:" << expected << std::endl;
    std::cout << "Returned Params:" << params << std::endl;
    std::cout << "Expected Wind:" << expectWind << std::endl;
    std::cout << "Returned Wind:" << windRequested << std::endl;

    EXPECT_EQUAL(expectWind, windRequested);
    EXPECT_EQUAL(params, expected);
}

void assertTypeExpansion(const std::string& name, std::vector<std::string> values,
                         const std::vector<std::string>& expected) {
    static MarsLanguage language("retrieve");
    MarsRequest req;
    req.setValuesTyped(language.type(name), values);
    req = language.expand(req, false, true);
    EXPECT_EQUAL(expected, req.values(name));
}

CASE("228") {
    assertTypeExpansion("param", {"129.228"}, {"228129"});
    assertTypeExpansion("param", {"228129"}, {"228129"});
    EXPECT_THROWS_AS(assertTypeExpansion("param", {"228003.228"}, {""}), eckit::UserError);
}

CASE("trivial") {

    std::vector<std::string> user   = {"1", "2", "3"};
    std::vector<std::string> axis   = {"1", "2", "3"};
    std::vector<std::string> expect = {"1", "2", "3"};

    test_param_axis(user, axis, expect, false);
}

CASE("wind1") {

    std::vector<std::string> user   = {"131", "132"};
    std::vector<std::string> axis   = {"138", "155"};
    std::vector<std::string> expect = {"131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}


CASE("wind2") {

    std::vector<std::string> user   = {"131", "132"};
    std::vector<std::string> axis   = {"131", "132", "138", "155"};
    std::vector<std::string> expect = {"131", "132"};

    test_param_axis(user, axis, expect, false);
}


CASE("wind3") {

    std::vector<std::string> user   = {"131", "132", "138", "155"};
    std::vector<std::string> axis   = {"138", "155"};
    std::vector<std::string> expect = {"131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}


CASE("wind4") {

    std::vector<std::string> user   = {"131.128", "132.128", "138.128", "155.128"};
    std::vector<std::string> axis   = {"138.128", "155.128"};
    std::vector<std::string> expect = {"131.128", "132.128", "138.128", "155.128"};

    test_param_axis(user, axis, expect, true);
}


CASE("wind5") {

    std::vector<std::string> user = {
        "131.128",
        "132.128",
    };
    std::vector<std::string> axis   = {"138", "155"};
    std::vector<std::string> expect = {"131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}


CASE("wind6") {

    std::vector<std::string> user   = {"131.128", "132.128", "138.128", "155.128"};
    std::vector<std::string> axis   = {"138", "155"};
    std::vector<std::string> expect = {"131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}


CASE("wind7") {

    std::vector<std::string> user   = {"131", "132", "138", "155"};
    std::vector<std::string> axis   = {"138.128", "155.128"};
    std::vector<std::string> expect = {"131.128", "132.128", "138.128", "155.128"};

    test_param_axis(user, axis, expect, true);
}


CASE("wind8") {

    std::vector<std::string> user = {
        "131",
        "132",
    };
    std::vector<std::string> axis   = {"138.128", "155.128"};
    std::vector<std::string> expect = {"131.128", "132.128", "138.128", "155.128"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind9") {

    std::vector<std::string> user = {
        "131",
        "132",
    };
    std::vector<std::string> axis   = {"138", "155", "210131"};
    std::vector<std::string> expect = {"131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind10") {

    std::vector<std::string> user = {
        "131",
        "132",
    };
    std::vector<std::string> axis   = {"129138", "129155"};
    std::vector<std::string> expect = {"129131", "129132", "129138", "129155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind11") {

    std::vector<std::string> user = {
        "131",
        "132",
    };
    std::vector<std::string> axis   = {"138", "155", "129138", "129155"};
    std::vector<std::string> expect = {"131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind12") {

    std::vector<std::string> user = {
        "131",
        "132.129",
    };
    std::vector<std::string> axis   = {"138", "155", "129138", "129155"};
    std::vector<std::string> expect = {"131", "129132", "138", "155", "129138", "129155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind13") {

    std::vector<std::string> user = {
        "131.128",
        "132.128",
    };
    std::vector<std::string> axis   = {"138", "155", "210131"};
    std::vector<std::string> expect = {"131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind14") {

    std::vector<std::string> user = {
        "131",
    };
    std::vector<std::string> axis   = {"138", "155", "210131"};
    std::vector<std::string> expect = {"131", "138", "155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind15") {

    std::vector<std::string> user = {
        "210131",
    };
    std::vector<std::string> axis   = {"138", "155", "210131"};
    std::vector<std::string> expect = {"210131"};

    test_param_axis(user, axis, expect, false);
}

CASE("wind16") {

    std::vector<std::string> user = {
        "210131",
        "131",
    };
    std::vector<std::string> axis   = {"138", "155", "210131"};
    std::vector<std::string> expect = {"210131", "131", "138", "155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind17") {

    std::vector<std::string> user = {
        "210131",
        "132",
    };
    std::vector<std::string> axis   = {"138", "155", "210131"};
    std::vector<std::string> expect = {"210131", "132", "138", "155"};

    test_param_axis(user, axis, expect, true);
}

CASE("wind18") {

    std::vector<std::string> user = {
        "210131",
        "131.128",
    };
    std::vector<std::string> axis   = {"138", "155", "210131"};
    std::vector<std::string> expect = {"210131", "131", "138", "155"};

    test_param_axis(user, axis, expect, true);
}
CASE("wind19") {

    std::vector<std::string> user = {
        "132",
    };
    std::vector<std::string> axis   = {"160138", "160155", "160131"};
    std::vector<std::string> expect = {"160132", "160138", "160155"};

    test_param_axis(user, axis, expect, true);
}
CASE("wind20") {

    std::vector<std::string> user = {
        "131",
    };
    std::vector<std::string> axis   = {"160138", "160155", "160131"};
    std::vector<std::string> expect = {"160131"};

    test_param_axis(user, axis, expect, false);
}
CASE("wind21") {

    std::vector<std::string> user = {
        "132",
    };
    std::vector<std::string> axis = {
        "120138",
        "120155",
        "170138",
        "170155",
    };
    std::vector<std::string> expect = {"170132", "170138", "170155"};

    test_param_axis(user, axis, expect, true);
}

CASE("mixed") {

    std::vector<std::string> user = {
        "129",
    };
    std::vector<std::string> axis   = {"129.128", "129"};
    std::vector<std::string> expect = {"129"};

    test_param_axis(user, axis, expect, false);
}

CASE("ocean1") {

    std::vector<std::string> user   = {"145"};
    std::vector<std::string> axis   = {"145.128", "164.128", "175.128", "148.128",
                                       "145.151", "164.151", "175.151", "148.151"};
    std::vector<std::string> expect = {"145.128"};

    test_param_axis(user, axis, expect, false);
}

CASE("ocean2") {

    std::vector<std::string> user   = {"145", "151145"};
    std::vector<std::string> axis   = {"145.128", "164.128", "175.128", "148.128",
                                       "145.151", "164.151", "175.151", "148.151"};
    std::vector<std::string> expect = {"145.128", "145.151"};

    test_param_axis(user, axis, expect, false);
}

CASE("ocean3") {

    std::vector<std::string> user   = {"145", "164", "175", "148", "151145"};
    std::vector<std::string> axis   = {"145.128", "164.128", "175.128", "148.128",
                                       "145.151", "164.151", "175.151", "148.151"};
    std::vector<std::string> expect = {"145.128", "164.128", "175.128", "148.128", "145.151"};

    test_param_axis(user, axis, expect, false);
}

CASE("ocean4") {

    std::vector<std::string> user   = {"145", "164", "175", "148"};
    std::vector<std::string> axis   = {"145.151", "164.151", "175.151", "148.151"};
    std::vector<std::string> expect = {"145.151", "164.151", "175.151", "148.151"};

    test_param_axis(user, axis, expect, false);
}

CASE("MARS-794 - GRIB1 and GRIB2 in axis") {

    // Note the paramId's at the end
    std::vector<std::string> axis = {
        "1.228",   "3.228",   "7.228",   "8.128",   "8.228",   "9.128",   "9.228",   "10.228",  "11.228",  "12.228",
        "13.228",  "14.228",  "20.3",    "20.128",  "21.228",  "22.228",  "23.228",  "24.228",  "26.128",  "29.228",
        "31.128",  "32.128",  "33.128",  "34.128",  "35.128",  "36.128",  "37.128",  "38.128",  "39.128",  "40.128",
        "41.128",  "42.128",  "44.128",  "44.228",  "45.128",  "46.228",  "47.128",  "47.228",  "48.228",  "49.128",
        "50.128",  "57.128",  "58.128",  "59.128",  "66.128",  "67.128",  "78.128",  "79.128",  "80.228",  "81.228",
        "82.228",  "83.228",  "84.228",  "85.228",  "88.228",  "89.228",  "90.228",  "98.174",  "129.128", "130.151",
        "131.151", "131.228", "132.151", "132.228", "134.128", "136.128", "137.128", "139.128", "141.128", "142.128",
        "143.128", "144.128", "145.128", "145.151", "146.128", "147.128", "148.128", "148.151", "151.128", "159.128",
        "163.151", "164.128", "164.151", "165.128", "166.128", "167.128", "168.128", "169.128", "170.128", "172.128",
        "175.128", "175.151", "176.128", "177.128", "178.128", "179.128", "180.128", "181.128", "182.128", "183.128",
        "186.128", "187.128", "188.128", "189.128", "195.128", "196.128", "197.128", "198.128", "201.128", "202.128",
        "205.128", "206.128", "208.128", "209.128", "210.128", "211.128", "212.128", "213.128", "216.228", "217.228",
        "218.228", "219.228", "220.228", "221.228", "226.228", "227.228", "228.128", "229.128", "230.128", "231.128",
        "232.128", "235.128", "236.128", "238.128", "239.228", "240.228", "241.228", "243.128", "244.128", "245.128",
        "246.228", "247.228", "251.228", "162071",  "162072",  "228050",  "228051",  "260015",  "260048",  "260109",
        "260121",  "260123",  "26.228",  "27.228",  "28.228",  "121.128", "122.128", "123.128", "222.228", "223.228",
        "224.228", "225.228", "228035",  "228036",  "228057",  "228058"};

    // by parameter xxx

    SECTION("by xxx only - priority to table 128") {
        std::vector<std::string> user   = {"148", "145", "164", "175"};
        std::vector<std::string> expect = {"148.128", "145.128", "164.128", "175.128"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by xxx and param.table") {
        std::vector<std::string> user   = {"148", "145.128", "164", "175.151"};
        std::vector<std::string> expect = {"148.128", "145.128", "164.128", "175.151"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by xxx and paramId") {
        std::vector<std::string> user   = {"148", "128145", "164", "151175"};
        std::vector<std::string> expect = {"148.128", "145.128", "164.128", "175.151"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by xxx and paramId and param.table") {
        std::vector<std::string> user   = {"148", "128145", "164", "175.151"};
        std::vector<std::string> expect = {"148.128", "145.128", "164.128", "175.151"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by xxx and paramId all ") {
        std::vector<std::string> user   = {"148", "145", "164", "175", "151148", "151145", "151164", "151175"};
        std::vector<std::string> expect = {"148.128", "145.128", "164.128", "175.128",
                                           "148.151", "145.151", "164.151", "175.151"};
        test_param_axis(user, axis, expect, false);
    }

    // by paramId

    SECTION("by paramId only 128") {
        std::vector<std::string> user   = {"128148", "128145", "128164", "128175"};
        std::vector<std::string> expect = {"148.128", "145.128", "164.128", "175.128"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by paramId only 151") {
        std::vector<std::string> user   = {"151148", "151145", "151164", "151175"};
        std::vector<std::string> expect = {"148.151", "145.151", "164.151", "175.151"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by paramId mixed 128 and 151") {
        std::vector<std::string> user   = {"151148", "128145", "128164", "151175"};
        std::vector<std::string> expect = {"148.151", "145.128", "164.128", "175.151"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by paramId all") {
        std::vector<std::string> user   = {"151148", "151145", "151164", "151175",
                                           "128148", "128145", "128164", "128175"};
        std::vector<std::string> expect = {"148.151", "145.151", "164.151", "175.151",
                                           "148.128", "145.128", "164.128", "175.128"};
        test_param_axis(user, axis, expect, false);
    }

    // by param.table

    SECTION("by param.table only 128") {
        std::vector<std::string> user   = {"148.128", "145.128", "164.128", "175.128"};
        std::vector<std::string> expect = {"148.128", "145.128", "164.128", "175.128"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by param.table only 151") {
        std::vector<std::string> user   = {"148.151", "145.151", "164.151", "175.151"};
        std::vector<std::string> expect = {"148.151", "145.151", "164.151", "175.151"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by param.table mixed 128 and 151") {
        std::vector<std::string> user   = {"148.128", "145.151", "164.128", "175.151"};
        std::vector<std::string> expect = {"148.128", "145.151", "164.128", "175.151"};
        test_param_axis(user, axis, expect, false);
    }
    SECTION("by param.table all") {
        std::vector<std::string> user   = {"148.151", "145.151", "164.151", "175.151",
                                           "148.128", "145.128", "164.128", "175.128"};
        std::vector<std::string> expect = {"148.151", "145.151", "164.151", "175.151",
                                           "148.128", "145.128", "164.128", "175.128"};
        test_param_axis(user, axis, expect, false);
    }
}


CASE("table1") {

    std::vector<std::string> user   = {"129", "130.128"};
    std::vector<std::string> axis   = {"129.128", "130"};
    std::vector<std::string> expect = {"129.128", "130"};

    test_param_axis(user, axis, expect, false);
}

CASE("table2") {

    std::vector<std::string> user   = {"129", "130.128"};
    std::vector<std::string> axis   = {"129.128", "130"};
    std::vector<std::string> expect = {"129.128", "130"};

    test_param_axis(user, axis, expect, false);
}

CASE("table3") {

    std::vector<std::string> user = {
        "129",
    };
    std::vector<std::string> axis = {
        "140129",
    };
    std::vector<std::string> expect = {"140129"};

    test_param_axis(user, axis, expect, false);
}

CASE("table4") {

    std::vector<std::string> user = {
        "129",
    };
    std::vector<std::string> axis = {
        "129.140",
    };
    std::vector<std::string> expect = {"129.140"};

    test_param_axis(user, axis, expect, false);
}

CASE("table5") {

    std::vector<std::string> user = {
        "129",
    };
    std::vector<std::string> axis   = {"129.128", "129.140"};
    std::vector<std::string> expect = {"129.128"};

    test_param_axis(user, axis, expect, false);
}

CASE("table6") {

    std::vector<std::string> user = {
        "129.128",
    };
    std::vector<std::string> axis   = {"129.128", "129.140"};
    std::vector<std::string> expect = {"129.128"};

    test_param_axis(user, axis, expect, false);
}

CASE("table7") {

    std::vector<std::string> user = {
        "129",
    };
    std::vector<std::string> axis   = {"129", "140129"};
    std::vector<std::string> expect = {"129"};

    test_param_axis(user, axis, expect, false);
}

CASE("table8") {

    std::vector<std::string> user = {
        "129.128",
    };
    std::vector<std::string> axis   = {"129", "140129"};
    std::vector<std::string> expect = {"129"};

    test_param_axis(user, axis, expect, false);
}


CASE("table9") {

    std::vector<std::string> user = {
        "129.128",
    };
    std::vector<std::string> axis   = {"129", "140129"};
    std::vector<std::string> expect = {"129"};

    test_param_axis(user, axis, expect, false);
}

CASE("table10") {

    std::vector<std::string> user   = {"131"};
    std::vector<std::string> axis   = {"210131"};
    std::vector<std::string> expect = {};

    test_param_axis(user, axis, expect, false, false);
    test_param_axis(user, axis, axis, false, true);
}

CASE("table11") {

    std::vector<std::string> user   = {"131"};
    std::vector<std::string> axis   = {"131.210"};
    std::vector<std::string> expect = {};

    test_param_axis(user, axis, expect, false, false);
    test_param_axis(user, axis, axis, false, true);
}

CASE("table12") {

    std::vector<std::string> user   = {"131", "132"};
    std::vector<std::string> axis   = {"210131", "170131", "180131", "160132"};
    std::vector<std::string> expect = {"170131", "160132"};

    test_param_axis(user, axis, expect, false);
}

CASE("table13") {

    std::vector<std::string> user   = {"131", "132"};
    std::vector<std::string> axis   = {"210131", "131.170", "180131", "160132"};
    std::vector<std::string> expect = {"131.170", "160132"};

    test_param_axis(user, axis, expect, false);
}

CASE("table14") {

    std::vector<std::string> user   = {"134", "133"};
    std::vector<std::string> axis   = {"210131", "133.170", "180134"};
    std::vector<std::string> expect = {"180134", "133.170"};

    test_param_axis(user, axis, expect, false);
}

CASE("table15") {

    std::vector<std::string> user   = {"134", "133.128"};
    std::vector<std::string> axis   = {"210131", "133.170", "180134"};
    std::vector<std::string> expect = {"180134"};

    test_param_axis(user, axis, expect, false);
}

}  // namespace metkit::mars::test

//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
