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



#include "eckit/exception/Exceptions.h"
#include "metkit/ParamID.h"
#include "metkit/MarsRequest.h"


#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
namespace test {

//-----------------------------------------------------------------------------
static std::ostream& operator<<(std::ostream& out, const std::vector<Param>& params) {
    out << '[';
    const char* sep = "";
    for(auto p : params) {
        out << sep << p;
        sep = ", ";
    }
    out << ']';
    return out;
}



static void test_param_axis(const std::vector<std::string>& user,
                 const std::vector<std::string>& axis,
                 const std::vector<std::string>& expect,
                 bool expectWind) {

    bool windRequested = false;

    MarsRequest ignore;

    std::vector<Param> params(user.begin(), user.end());
    std::vector<Param> result(expect.begin(), expect.end());
    std::vector<Param> index(axis.begin(), axis.end());

    std::sort(index.begin(), index.end());


    std::cout << "User:" << params << std::endl;
    std::cout << "Axis:" << index << std::endl;
    std::cout << "Wind:" << false << std::endl;


    ParamID::normalise(ignore, params, index, windRequested);

    std::cout << "Result:" << params << " wind=" << windRequested <<  std::endl;


    EXPECT(expectWind == windRequested);
    EXPECT(params == result);
}


CASE ("trivial") {

    std::vector<std::string> user = {"1", "2", "3"};
    std::vector<std::string> axis = {"1", "2", "3"};
    std::vector<std::string> expect = {"1", "2", "3"};

    test_param_axis(user, axis, expect, false);

}

CASE ("wind1") {

    std::vector<std::string> user = {"131", "132"};
    std::vector<std::string> axis = {"138", "155"};
    std::vector<std::string> expect = {"138", "155"};

    test_param_axis(user, axis, expect, true);


}


CASE ("wind2") {

    std::vector<std::string> user = {"131", "132"};
    std::vector<std::string> axis = {"131", "132", "138", "155"};
    std::vector<std::string> expect = {"131", "132"};

    test_param_axis(user, axis, expect, false);


}


CASE ("wind3") {

    std::vector<std::string> user = {"131", "132", "138", "155"};
    std::vector<std::string> axis = {"138", "155"};
    std::vector<std::string> expect = {"138", "155"};

    test_param_axis(user, axis, expect, true);


}


CASE ("table1") {

    std::vector<std::string> user = {"129", "130.128"};
    std::vector<std::string> axis = {"129.128", "130"};
    std::vector<std::string> expect = {"129.128", "130"};

    test_param_axis(user, axis, expect, false);


}



CASE ("table2") {

    std::vector<std::string> user = {"129", "130.128"};
    std::vector<std::string> axis = {"129.128", "130"};
    std::vector<std::string> expect = {"129.128", "130"};

    test_param_axis(user, axis, expect, false);


}



CASE ("table3") {

    std::vector<std::string> user = {"129", };
    std::vector<std::string> axis = {"140129", };
    std::vector<std::string> expect = {"140129"};

    test_param_axis(user, axis, expect, false);


}


CASE ("table4") {

    std::vector<std::string> user = {"129", };
    std::vector<std::string> axis = {"129.140", };
    std::vector<std::string> expect = {"129.140"};

    test_param_axis(user, axis, expect, false);


}



}  // namespace test
}  // namespace metkit

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
