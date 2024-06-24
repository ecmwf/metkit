/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date   June 2024
/// @author Philipp Geier



#include "metkit/mars2grib/Mars2Grib.h"
#include "metkit/mars2grib/ValueMapSetter.h"

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"


using namespace eckit::testing;

namespace metkit::mars2grib::test {

//----------------------------------------------------------------------------------------------------------------------

CASE("test pdt selection") {
    {
        eckit::ValueMap initial{};
        eckit::ValueMap out;
        std::deque<std::string> keys;
        
        OrderedValueMapSetter setter{out, keys};
        
        convertMars2Grib(initial, setter);
        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchPDT = out.find("productDefinitionTemplateNumber");
        EXPECT(searchPDT != out.end());
        EXPECT_EQUAL(static_cast<long>(searchPDT->second), 0);
    }
    
    {
        eckit::ValueMap initial{{"paramId", 8}};
        eckit::ValueMap out;
        std::deque<std::string> keys;
        
        OrderedValueMapSetter setter{out, keys};

        convertMars2Grib(initial, setter);
        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchPDT = out.find("productDefinitionTemplateNumber");
        EXPECT(searchPDT != out.end());
        EXPECT_EQUAL(static_cast<long>(searchPDT->second), 8);
    }
}


CASE("test level mapping") {
    {
        eckit::ValueMap initial{{"paramId", 260367}, {"levtype", "sol"}, {"level", 4}};
        eckit::ValueMap out;
        std::deque<std::string> keys;

        OrderedValueMapSetter setter{out, keys};

        convertMars2Grib(initial, setter);
        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchTypeOfLevel = out.find("typeOfLevel");
        EXPECT(searchTypeOfLevel != out.end());
        EXPECT_EQUAL(static_cast<std::string>(searchTypeOfLevel->second), "soilLayer");
        auto searchFirstSurface = out.find("scaledValueOfFirstFixedSurface");
        EXPECT(searchFirstSurface != out.end());
        EXPECT_EQUAL(static_cast<long>(searchFirstSurface->second), 3);
        auto searchSecondSurface = out.find("scaledValueOfSecondFixedSurface");
        EXPECT(searchSecondSurface != out.end());
        EXPECT_EQUAL(static_cast<long>(searchSecondSurface->second), 4);
    }
    
    {
        eckit::ValueMap initial{{"paramId", 260644}, {"levtype", "sol"}, {"level", 4}};
        eckit::ValueMap out;
        std::deque<std::string> keys;

        OrderedValueMapSetter setter{out, keys};

        convertMars2Grib(initial, setter);
        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchTypeOfLevel = out.find("typeOfLevel");
        EXPECT(searchTypeOfLevel != out.end());
        EXPECT_EQUAL(static_cast<std::string>(searchTypeOfLevel->second), "soil");
        auto searchFirstSurface = out.find("scaledValueOfFirstFixedSurface");
        EXPECT(searchFirstSurface != out.end());
        EXPECT_EQUAL(static_cast<long>(searchFirstSurface->second), 4);
        auto searchSecondSurface = out.find("scaledValueOfSecondFixedSurface");
        EXPECT(searchSecondSurface == out.end());
    }
}





//----------------------------------------------------------------------------------------------------------------------

}  // namespace test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
