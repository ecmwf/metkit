/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

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
    EXPECT(ss.str() == std::string("[any,date,enum,expver,float,integer,lowercase,mixed,param,range,regex,time,to-by-"
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
