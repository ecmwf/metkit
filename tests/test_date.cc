/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_date.cc
/// @author Emanuele Danovaro
/// @date   March 2025

#include "eckit/value/Value.h"
#include "eckit/types/Date.h"
#include "eckit/testing/Test.h"

#include "metkit/mars/TypeDate.h"
#include "metkit/mars/MarsExpandContext.h"

using namespace eckit;
using namespace eckit::testing;

namespace metkit::mars::test {

//-----------------------------------------------------------------------------

CASE("Test TypeDate expansions") {

    TypeDate tdate("date", Value());
    Type& td(tdate);
    DummyContext ctx;

    // 1 and 2-digit times

    {
        std::string value = "20140506";
        td.expand(ctx, value);
        EXPECT(value == "20140506");
    }
    {
        std::string value = "2014-05-06";
        td.expand(ctx, value);
        EXPECT(value == "20140506");
    }
    {
        std::string value = "20141506";
        EXPECT_THROWS(td.expand(ctx, value)); // throws BadDate that is not exported
    }
    {
        std::string value = "-1";
        td.expand(ctx, value);
        // EXPECT(value == "0600");
    }
    {
        std::vector<std::string> values{"20140506", "20140507"};
        td.expand(ctx, values);
        EXPECT(values.size() == 2);
        EXPECT(values[0] == "20140506");
        EXPECT(values[1] == "20140507");
    }
    {
        std::vector<std::string> values{"20140506", "to", "20140506"};
        td.expand(ctx, values);
        EXPECT(values.size() == 1);
        EXPECT(values[0] == "20140506");
    }
    {
        std::vector<std::string> values{"20140506", "to", "20140507"};
        td.expand(ctx, values);
        EXPECT(values.size() == 2);
        EXPECT(values[0] == "20140506");
        EXPECT(values[1] == "20140507");
    }
    {
        std::vector<std::string> values{"20140506", "to", "20140508"};
        td.expand(ctx, values);
        EXPECT(values.size() == 3);
        EXPECT(values[0] == "20140506");
        EXPECT(values[1] == "20140507");
        EXPECT(values[2] == "20140508");
    }
    {
        std::vector<std::string> values{"20140504", "20140506", "to", "20140508"};
        td.expand(ctx, values);
        EXPECT(values.size() == 4);
        EXPECT(values[0] == "20140504");
        EXPECT(values[1] == "20140506");
        EXPECT(values[2] == "20140507");
        EXPECT(values[3] == "20140508");
    }
}


//-----------------------------------------------------------------------------

}  // namespace metkit::mars::test

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
