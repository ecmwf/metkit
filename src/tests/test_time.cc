/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @file   test_time.cc
/// @author Simon Smart
/// @date   April 2020

#include "eckit/value/Value.h"
#include "eckit/testing/Test.h"

#include "metkit/types/TypeTime.h"
#include "metkit/MarsExpandContext.h"

using namespace eckit;
using namespace eckit::testing;

namespace metkit {
namespace test {

//-----------------------------------------------------------------------------

CASE("Test TypeType expansions") {

    TypeTime ttime("time", Value());
    Type& tt(ttime);
    DummyContext ctx;

    // 2-digit times

    {
        std::string value = "00";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "12";
        tt.expand(ctx, value);
        EXPECT(value == "1200");
    }

    // 4-digit times

    {
        std::string value = "0000";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "0012";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "1234";
        tt.expand(ctx, value);
        EXPECT(value == "1234");
    }

    // 4-digit times with colons

    {
        std::string value = "00:00";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "00:12";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "12:34";
        tt.expand(ctx, value);
        EXPECT(value == "1234");
    }

    // 6-digit times

    {
        std::string value = "000000";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "000012";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "001200";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "123456";
        tt.expand(ctx, value);
        EXPECT(value == "1234");
    }

    // 6-digit times with colons

    {
        std::string value = "00:00:00";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "00:00:12";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "00:12:00";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "12:34:56";
        tt.expand(ctx, value);
        EXPECT(value == "1234");
    }
}


//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
