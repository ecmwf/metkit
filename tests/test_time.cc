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

#include "eckit/testing/Test.h"
#include "eckit/value/Value.h"

#include "metkit/mars/MarsExpandContext.h"
#include "metkit/mars/TypeTime.h"

using namespace eckit;
using namespace eckit::testing;

namespace metkit {
namespace mars {
namespace test {

//-----------------------------------------------------------------------------

CASE("Test TypeTime expansions") {

    TypeTime ttime("time", Value());
    Type& tt(ttime);
    DummyContext ctx;

    // 1 and 2-digit times

    {
        std::string value = "0";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
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
    {
        std::string value = "6";
        tt.expand(ctx, value);
        EXPECT(value == "0600");
    }
    {
        std::string value = "06";
        tt.expand(ctx, value);
        EXPECT(value == "0600");
    }

    // 3 and 4-digit times

    {
        std::string value = "000";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "0000";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "012";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
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
    {
        std::string value = "623";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        std::string value = "0623";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        std::string value = "675";
        EXPECT_THROWS_AS(tt.expand(ctx, value), BadTime);
    }

    // 5 and 6-digit times

    {
        std::string value = "000000";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        // We don't support seconds yet.
        std::string value = "000012";
        EXPECT_THROWS_AS(tt.expand(ctx, value), SeriousBug);
    }
    {
        std::string value = "001200";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "123400";
        tt.expand(ctx, value);
        EXPECT(value == "1234");
    }
    {
        std::string value = "062300";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        std::string value = "62300";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        // We don't support seconds yet.
        std::string value = "123456";
        EXPECT_THROWS_AS(tt.expand(ctx, value), SeriousBug);
    }

    // times with colons

    {
        std::string value = "0:0";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "00:00";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "00:00:00";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "0:12";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "00:12";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "00:62";
        EXPECT_THROWS_AS(tt.expand(ctx, value), BadTime);
    }
    {
        std::string value = "12:34";
        tt.expand(ctx, value);
        EXPECT(value == "1234");
    }
    {
        std::string value = "6:23";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        std::string value = "06:23";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        // We don't support seconds yet.
        std::string value = "00:00:12";
        EXPECT_THROWS_AS(tt.expand(ctx, value), SeriousBug);
    }
    {
        std::string value = "00:12:00";
        tt.expand(ctx, value);
        EXPECT(value == "0012");
    }
    {
        std::string value = "12:34:00";
        tt.expand(ctx, value);
        EXPECT(value == "1234");
    }
    {
        std::string value = "06:23:00";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        std::string value = "6:23:00";
        tt.expand(ctx, value);
        EXPECT(value == "0623");
    }
    {
        // We don't support seconds yet.
        std::string value = "12:34:56";
        EXPECT_THROWS_AS(tt.expand(ctx, value), SeriousBug);
    }

    // times with units

    {
        std::string value = "0h";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "00H";
        tt.expand(ctx, value);
        EXPECT(value == "0000");
    }
    {
        std::string value = "60m";
        tt.expand(ctx, value);
        EXPECT(value == "0100");
    }
    {
        std::string value = "2h30m";
        tt.expand(ctx, value);
        EXPECT(value == "0230");
    }
}


//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace mars
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
