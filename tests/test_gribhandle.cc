/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date   Apr 2024
/// @author Christopher Bradley

#include "eccodes.h"

#include "eckit/io/FileHandle.h"
#include "eckit/testing/Test.h"

#include "metkit/codes/GribAccessor.h"
#include "metkit/codes/GribHandle.h"

using namespace eckit::testing;

namespace metkit {
namespace grib {
namespace test {

//-----------------------------------------------------------------------------

// Test that a gribhandle will point to the correct message in a file, given an offset.
CASE("File with two messages") {

    // The test file has two messages of different packing types, with some junk data in between.
    eckit::PathName path("synthetic_2msgs.grib");

    off_t* offsets;
    grib_context* c = nullptr;
    int n           = 0;
    int err         = codes_extract_offsets_malloc(c, path.asString().c_str(), PRODUCT_GRIB, &offsets, &n, 1);
    EXPECT(!err);
    EXPECT(n == 2);

    GribAccessor<std::string> packingType("packingType");
    std::vector<std::string> expected = {"grid_simple", "grid_ccsds"};

    eckit::FileHandle dh(path);
    dh.openForRead();

    for (int i = 0; i < n; i++) {
        GribHandle h(dh, offsets[i]);
        EXPECT(packingType(h) == expected[i]);
    }

    free(offsets);
    dh.close();
}

//-----------------------------------------------------------------------------

}  // namespace test
}  // namespace grib
}  // namespace metkit

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
