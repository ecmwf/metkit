// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   test_typesfactory.cc
/// @author Simon Smart
/// @date   April 2017

#include <iostream>
#include <sstream>
#include <string>

#include "eckit/testing/Test.h"
#include "eckit/value/Value.h"

#include "metkit/mars/TypeDate.h"
#include "metkit/mars/TypesFactory.h"

namespace metkit::mars::test {

//-----------------------------------------------------------------------------


CASE("test_list_types") {

    std::stringstream ss;
    TypesFactory::list(ss);
    std::cout << ss.str() << std::endl;
    EXPECT_EQUAL(ss.str(),
                 std::string("[any,date,enum,expver,float,integer,lowercase,mixed,param,range,regex,time,to-by-"
                             "list,to-by-list-float,to-by-list-quantile]"));
}


CASE("test_build") {

    eckit::ValueMap settings;
    settings["type"] = "date";

    Type* t1(TypesFactory::build("abcd", eckit::Value(settings)));

    EXPECT(t1 != 0);
    t1->attach();

    // Check that we have obtained the correct type
    EXPECT(dynamic_cast<TypeDate*>(t1) != 0);

    // Clean up, taking into account that ~Type is protected.
    t1->detach();
}

}  // namespace metkit::mars::test


//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
