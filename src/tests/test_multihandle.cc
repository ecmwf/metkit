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

#include "eckit/filesystem/PathName.h"
#include "eckit/io/Buffer.h"

#include "eckit/io/MultiHandle.h"
#include "eckit/io/FileHandle.h"

#include "metkit/grib/MetFile.h"

#include "grib_api.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
namespace grib {
namespace test {

//----------------------------------------------------------------------------------------------------------------------

CASE( "fopen" ) {

    eckit::MultiHandle mh;

    mh += new eckit::FileHandle("latlon.grib");
    mh += new eckit::FileHandle("latlon.grib");

    FILE* f = mh.openf("r");

    grib_handle* h;

    int err = 0;
    size_t count = 0;
    while( (h =  grib_handle_new_from_file(0, f, &err))) {
        count++;
        grib_handle_delete(h);
    }

    fclose(f);
    EXPECT(count == 2);
    EXPECT(err == GRIB_SUCCESS);
}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace grib
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
