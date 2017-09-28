/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "metkit/grib/MetFile.h"

#include "eckit/testing/Test.h"

using namespace eckit::testing;

namespace metkit {
namespace grib {
namespace test {

//----------------------------------------------------------------------------------------------------------------------

static const size_t GRIB_SIZE = 858;

struct Fixture {
    Fixture() : file("latlon.grib") {}
    MetFile file;
};

//----------------------------------------------------------------------------------------------------------------------

CASE ( "metkit_grib_MetFile" ) {

    SETUP() {
        Fixture F;

        SECTION( "test_read" ) {
            eckit::Buffer buf(1024);
            size_t len = F.file.read(buf);
            EXPECT(len < buf.size());
            EXPECT(len == GRIB_SIZE);
        }

        SECTION( "test_read_some" ) {
            eckit::Buffer buf(1024);
            size_t len = F.file.readSome(buf);
            EXPECT(len < buf.size());
            EXPECT(len == GRIB_SIZE);
        }

        SECTION( "test_read_some_smallbuff" ) {
            eckit::Buffer buf(512);
            size_t len = F.file.readSome(buf);
            EXPECT(len > buf.size());
            EXPECT(len == GRIB_SIZE);
        }
    }

}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace grib
}  // namespace metkit

int main(int argc, char **argv)
{
    return run_tests ( argc, argv );
}
