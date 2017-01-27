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

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"

#include "eckit/testing/Setup.h"

#include "metkit/grib/EmosFile.h"

using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup);

namespace metkit {
namespace grib {
namespace test {

//----------------------------------------------------------------------------------------------------------------------

static const size_t GRIB_SIZE = 858;

struct F {
    F() : file("latlon.grib") {}
    EmosFile file;
};

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( metkit_grib_EmosFile )

BOOST_FIXTURE_TEST_CASE( test_read, F ) {
    Buffer buf(1024);
    size_t len = file.read(buf);
    BOOST_CHECK_LT(len, buf.size());
    BOOST_CHECK_EQUAL(len, GRIB_SIZE);
}

BOOST_FIXTURE_TEST_CASE( test_read_some, F ) {
    Buffer buf(1024);
    size_t len = file.readSome(buf);
    BOOST_CHECK_LT(len, buf.size());
    BOOST_CHECK_EQUAL(len, GRIB_SIZE);
}

BOOST_FIXTURE_TEST_CASE( test_read_some_smallbuff, F ) {
    Buffer buf(512);
    size_t len = file.readSome(buf);
    BOOST_CHECK_GT(len, buf.size());
    BOOST_CHECK_EQUAL(len, GRIB_SIZE);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace grib
} // namespace metkit

