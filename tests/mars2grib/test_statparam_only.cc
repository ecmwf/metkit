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

CASE("test stat param") {
    {
        eckit::ValueMap initial{{"paramId", 7}};
        eckit::ValueMap out;
        std::deque<std::string> keys;
        
        OrderedValueMapSetter setter{out, keys, true};
        
        convertMars2Grib(initial, setter, statParamRuleList());
        
        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchTOSP = out.find("typeOfStatisticalProcessing");
        EXPECT(searchTOSP == out.end());
    }
    
    {
        eckit::ValueMap initial{{"paramId", 8}};
        eckit::ValueMap out;
        std::deque<std::string> keys;

        OrderedValueMapSetter setter{out, keys, true};

        convertMars2Grib(initial, setter, statParamRuleList());

        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchTOSP = out.find("typeOfStatisticalProcessing");
        EXPECT(searchTOSP != out.end());
        EXPECT_EQUAL(static_cast<long>(searchTOSP->second), 1);
    }
    
    {
        eckit::ValueMap initial{{"paramId", 49}};
        eckit::ValueMap out;
        std::deque<std::string> keys;

        OrderedValueMapSetter setter{out, keys, true};

        convertMars2Grib(initial, setter, statParamRuleList());

        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchTOSP = out.find("typeOfStatisticalProcessing");
        EXPECT(searchTOSP != out.end());
        EXPECT_EQUAL(static_cast<long>(searchTOSP->second), 2);
        auto searchLOTR = out.find("lengthOfTimeRange");
        EXPECT(searchLOTR == out.end());
        auto searchIUFR = out.find("indicatorOfUnitForTimeRange");
        EXPECT(searchIUFR != out.end());
        EXPECT_EQUAL(static_cast<long>(searchIUFR->second), 1);
    }
    
    {
        eckit::ValueMap initial{{"paramId", 51}};
        eckit::ValueMap out;
        std::deque<std::string> keys;

        OrderedValueMapSetter setter{out, keys, true};

        convertMars2Grib(initial, setter, statParamRuleList());

        eckit::Log::info() << "Mapped: " << initial << " to " << setter << std::endl;
        auto searchTOSP = out.find("typeOfStatisticalProcessing");
        EXPECT(searchTOSP != out.end());
        EXPECT_EQUAL(static_cast<long>(searchTOSP->second), 2);
        auto searchLOTR = out.find("lengthOfTimeRange");
        EXPECT(searchLOTR != out.end());
        EXPECT_EQUAL(static_cast<long>(searchLOTR->second), 24);
        auto searchIUFR = out.find("indicatorOfUnitForTimeRange");
        EXPECT(searchIUFR != out.end());
        EXPECT_EQUAL(static_cast<long>(searchIUFR->second), 1);
    }
}



//----------------------------------------------------------------------------------------------------------------------

}  // namespace test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
