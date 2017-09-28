/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "metkit/MarsRequest.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
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

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}