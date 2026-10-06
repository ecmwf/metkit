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

#include "metkit/mars/MarsLanguage.h"
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

    auto t1 = TypesFactory::build(MarsLanguage::addKeyword("abcd"), eckit::Value(settings));

    EXPECT(t1 != 0);

    // Check that we have obtained the correct type
    EXPECT(dynamic_cast<TypeDate*>(t1.get()) != 0);
}

CASE("test_enum_values") {

    // [name, alias...]: aliases containing blanks are descriptions and are not values, the name always is a value
    eckit::ValueList values;
    values.push_back(eckit::Value(eckit::ValueList{eckit::Value("on-demand-extremes-dt"),
                                                   eckit::Value("On-demand weather and geophysical extremes twin")}));
    values.push_back(eckit::Value(eckit::ValueList{eckit::Value("monthly run")}));
    values.push_back(eckit::Value(eckit::ValueList{eckit::Value("bc"), eckit::Value("boundary conditions")}));

    eckit::ValueMap settings;
    settings["type"]   = "enum";
    settings["values"] = eckit::Value(values);

    auto t = TypesFactory::build(MarsLanguage::addKeyword("testenum"), eckit::Value(settings));
    MarsRequest request;

    auto expand = [&](std::string value) {
        bool ok = t->expand(value, request);
        return std::make_pair(ok, value);
    };

    // long names are values (there is no limit on the length)
    EXPECT(expand("on-demand-extremes-dt") == std::make_pair(true, std::string("on-demand-extremes-dt")));
    // a name containing a blank is still a value, and matching is case insensitive
    EXPECT(expand("Monthly Run") == std::make_pair(true, std::string("monthly run")));
    EXPECT(expand("bc") == std::make_pair(true, std::string("bc")));
    // descriptions are not
    EXPECT(!expand("boundary conditions").first);
    EXPECT(!expand("not-a-value").first);
}

}  // namespace metkit::mars::test


//-----------------------------------------------------------------------------

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
