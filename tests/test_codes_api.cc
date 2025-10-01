/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "eckit/filesystem/PathName.h"
#include "eckit/io/FileHandle.h"
#include "eckit/testing/Test.h"


#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/OwningCodesHandle.h"

namespace metkit::grib::test {

//-----------------------------------------------------------------------------

CASE("Test load and iterate sample") {
    using namespace codes;

    OwningCodesHandle handle(newFromSample("GRIB2"));

    for (auto& k : handle.keys(Namespace::Mars)) {
        std::cout << k.name() << ": ";
        std::visit([](auto&& v) { std::cout << v << std::endl; }, k.get());
    }

    for (auto d : handle.values()) {
        std::cout << d.longitude << "/" << d.latitude << ": " << d.value << std::endl;
    }
}

//-----------------------------------------------------------------------------

}  // namespace metkit::grib::test


int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
