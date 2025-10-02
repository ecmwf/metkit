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

CASE("Print version and paths") {
    std::cout << codes::info() << std::endl;
    std::cout << "samplesPath: " << codes::samplesPath() << std::endl;
    std::cout << "definitionPath: " << codes::definitionPath() << std::endl;
    
    EXPECT_EQUAL(codes::samplesPath(), "/MEMFS/samples");
    EXPECT_EQUAL(codes::definitionPath(), "/MEMFS/definitions");
}

CASE("Test load and iterate sample") {
    using namespace codes;

    OwningCodesHandle handle(newFromSample("GRIB2"));
    
    // handle.set("date", "20250101");
    // handle.set("time", "1400");
    // handle.set("step", (long) 18);
    // handle.set("levtype", "pl");
    // handle.set("param", (long) 132);

    for (auto& k : handle.keys(namespaces::mars)) {
        std::cout << k.name() << ": ";
        std::visit([](auto&& v) { std::cout << v << std::endl; }, k.get());
        
        // if (k.name() == "date") {
        //     EXPECT_EQUAL(std::get<long>(k.get()), 20250101);
        // }
        // if (k.name() == "time") {
        //     EXPECT_EQUAL(std::get<long>(k.get()), 1400);
        // }
        // if (k.name() == "step") {
        //     EXPECT_EQUAL(std::get<long>(k.get()), 18);
        // }
        // if (k.name() == "levtype") {
        //     EXPECT_EQUAL(std::get<std::string>(k.get()), "pl");
        // }
        // if (k.name() == "param") {
        //     EXPECT_EQUAL(std::get<std::string>(k.get()), "132");
        // }
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
