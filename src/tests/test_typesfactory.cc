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

#include "eckit/exception/Exceptions.h"
#include "eckit/runtime/Tool.h"

using namespace eckit;
using namespace metkit;

//----------------------------------------------------------------------------------------------------------------------

// We use eckit::Tool for the tests as it ensures that the Main environment is set up
// correctly, avoiding any unexpected segfaults inside eckit.

class TypesFactoryTest : public Tool {

public: // methods

    TypesFactoryTest(int argc, char** argv);
    virtual ~TypesFactoryTest();

    virtual void run();

private: // methods

    void test_list_types();

};


TypesFactoryTest::TypesFactoryTest(int argc, char** argv) :
    Tool(argc, argv) {}


TypesFactoryTest::~TypesFactoryTest() {}


void TypesFactoryTest::test_list_types() {

    std::stringstream ss;
    TypesFactory::list(ss);
    ASSERT(ss.str() == std::string("[any,date,enum,enum-or-more,expver,float,integer,param,range,time,to-by-list]"));
}


void TypesFactoryTest::run() {
    test_list_types();
}


int main(int argc, char** argv) {
    TypesFactoryTest tests(argc, argv);
    return tests.start();
}

//----------------------------------------------------------------------------------------------------------------------
