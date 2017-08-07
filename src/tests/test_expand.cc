/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_emosfile.cc
/// @date   Jan 2016
/// @author Florian Rathgeber

#define BOOST_TEST_MODULE metkit_grib_EmosFile
#include "ecbuild/boost_test_framework.h"


#include "eckit/testing/Setup.h"

#include "metkit/MarsRequest.h"

using namespace eckit;
using namespace eckit::testing;

using namespace metkit;

BOOST_GLOBAL_FIXTURE(Setup);

//----------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( metkit_expand )

BOOST_AUTO_TEST_CASE( test_metkit_expand ) {
   const char* text = "ret,date=-5/to/-1";
   MarsRequest r = MarsRequest::parse(text);
   r.dump(std::cout);
}

BOOST_AUTO_TEST_SUITE_END()


