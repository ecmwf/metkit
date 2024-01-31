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

#include "metkit/mars/StepRange.h"
#include "metkit/mars/StepRangeNormalise.h"


#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace eckit {
    template <> struct VectorPrintSelector<metkit::mars::StepRange> { typedef VectorPrintSimple selector; };
}

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

CASE("steprange") {

    {
        StepRange sr{0,24};
        EXPECT(sr.from()==0);
        std::cout << sr.to() << std::endl;
        EXPECT(sr.to()==24);
    }
    {
        StepRange sr{"0-24"};
        EXPECT(sr.from()==0);
        EXPECT(sr.to()==24);
    }
    {
        StepRange sr{0,.5};
        EXPECT(sr.from()==0);
        EXPECT(sr.to()==0.5);
    }
    {
        StepRange sr{"0-30m"};
        EXPECT(sr.from()==0);
        EXPECT(sr.to()==0.5);
    }
    {
        StepRange sr{"0-24s"};
        EXPECT(sr.from()==0);
        EXPECT(sr.to()==(24./3600.));
    }
}


static void test_steprange_axis(
                 const std::vector<std::string>& user,
                 const std::vector<std::string>& axis,
                 const std::vector<std::string>& expect) {

    std::vector<StepRange> values(user.begin(), user.end());
    std::vector<StepRange> result(expect.begin(), expect.end());
    std::vector<StepRange> index(axis.begin(), axis.end());

    std::sort(index.begin(), index.end());


    std::cout << "User:" << values << std::endl;
    std::cout << "Axis:" << index << std::endl;

    StepRangeNormalise::normalise(values, index);

    std::cout << "Result:" << values <<  std::endl;

    EXPECT(values == result);
}


CASE("trivial") {

    std::vector<std::string> user = {"1", "2", "3"};
    std::vector<std::string> axis = {"1", "2", "3"};
    std::vector<std::string> expect = {"1", "2", "3"};

    test_steprange_axis(user, axis, expect);
}

CASE("subselection") {

    std::vector<std::string> user = {"2", "3"};
    std::vector<std::string> axis = {"1", "2", "3"};
    std::vector<std::string> expect = {"2", "3"};

    test_steprange_axis(user, axis, expect);
}

CASE("missing values") {
    std::vector<std::string> user = {"1", "2", "3"};
    std::vector<std::string> axis = {"1", "3"};
    std::vector<std::string> expect = {"1", "3"};

    test_steprange_axis(user, axis, expect);
}

CASE("ranges") {
    std::vector<std::string> user = {"0-24", "24-48", "3-9"};
    std::vector<std::string> axis = {"0-24", "6-30", "12-36", "18-42", "24-48"};
    std::vector<std::string> expect = {"0-24", "24-48"};

    test_steprange_axis(user, axis, expect);
}

CASE("default start-point") {
    std::vector<std::string> user = {"1", "2", "24", "25"};
    std::vector<std::string> axis = {"1", "0-1", "3", "0-3", "0-24", "25"};
    std::vector<std::string> expect = {"1", "0-1", "0-24", "25"};

    test_steprange_axis(user, axis, expect);
}

CASE("match range start") {
    // SDS: I'm not really sure why this is supported, but the original
    //      MARS code did it...
    std::vector<std::string> user = {"2-24"};
    std::vector<std::string> axis = {"1", "2", "3"};
    std::vector<std::string> expect = {"2"};

    test_steprange_axis(user, axis, expect);
}


}  // namespace test
}  // namespace mars
}  // namespace metkit

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
