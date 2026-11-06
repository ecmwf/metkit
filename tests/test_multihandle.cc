// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @file   test_MetFile.cc
/// @date   Jan 2016
/// @author Florian Rathgeber

#include "eckit/io/FileHandle.h"
#include "eckit/io/MultiHandle.h"
#include "eckit/testing/Test.h"

#include "eccodes.h"

namespace metkit::grib::test {

//----------------------------------------------------------------------------------------------------------------------

CASE("openf filehandle") {

    eckit::FileHandle dh("latlon.grib");

    FILE* f = dh.openf("r");

    codes_handle* h;

    int err      = 0;
    size_t count = 0;
    while ((h = codes_handle_new_from_file(nullptr, f, PRODUCT_ANY, &err))) {
        count++;
        codes_handle_delete(h);
    }

    fclose(f);
    EXPECT_EQUAL(count, 1);
    EXPECT_EQUAL(err, GRIB_SUCCESS);
}

//----------------------------------------------------------------------------------------------------------------------

CASE("openf multihandle") {

    eckit::MultiHandle dh;
    dh += new eckit::FileHandle("latlon.grib");
    dh += new eckit::FileHandle("latlon.grib");

    FILE* f = dh.openf("r");

    codes_handle* h;

    int err      = 0;
    size_t count = 0;
    while ((h = codes_handle_new_from_file(nullptr, f, PRODUCT_ANY, &err))) {
        count++;
        codes_handle_delete(h);
    }

    fclose(f);
    EXPECT_EQUAL(count, 2);
    EXPECT_EQUAL(err, GRIB_SUCCESS);
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
