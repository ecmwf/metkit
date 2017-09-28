/*
 * (C) Copyright 1996-2017 ECMWF.
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

#include "metkit/types/TypesFactory.h"
#include "metkit/types/TypeDate.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Tool.h"
#include "eckit/value/Value.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
namespace test {

//-----------------------------------------------------------------------------


CASE ("test_list_types") {

    std::stringstream ss;
    TypesFactory::list(ss);
    std::cout << ss.str() << std::endl;
    EXPECT(ss.str() == std::string("[any,date,enum,expver,float,integer,mixed,param,range,regex,time,to-by-list]"));
}


CASE ("test_build") {

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

}  // namespace test
}  // namespace metkit

//-----------------------------------------------------------------------------

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}