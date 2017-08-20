/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @file   test_MetFile.cc
/// @date   Jan 2016
/// @author Florian Rathgeber

#define BOOST_TEST_MODULE metkit_grib_multihandle
#include "ecbuild/boost_test_framework.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"

#include "eckit/io/MultiHandle.h"
#include "eckit/io/FileHandle.h"

#include "eckit/testing/Setup.h"

#include "metkit/grib/MetFile.h"

#include "grib_api.h"

using namespace eckit;
using namespace eckit::testing;

BOOST_GLOBAL_FIXTURE(Setup);

namespace metkit {
namespace grib {
namespace test {

//----------------------------------------------------------------------------------------------------------------------


struct F {
};

//----------------------------------------------------------------------------------------------------------------------

BOOST_AUTO_TEST_SUITE( metkit_grib_multihandle )

BOOST_FIXTURE_TEST_CASE( fopen, F ) {

    MultiHandle mh;

    mh += new FileHandle("latlon.grib");
    mh += new FileHandle("latlon.grib");

    FILE* f = mh.openf("r");

    grib_handle* h;

    int err = 0;
    size_t count = 0;
    while( (h =  grib_handle_new_from_file(0, f, &err))) {
        count++;
        grib_handle_delete(h);
    }

    fclose(f);
    BOOST_CHECK_EQUAL(count, 2);
    BOOST_CHECK_EQUAL(err, GRIB_SUCCESS);
}

BOOST_AUTO_TEST_SUITE_END()

} // namespace test
} // namespace grib
} // namespace metkit

