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

#include "eckit/config/LocalConfiguration.h"
#include "eckit/testing/Test.h"


using namespace eckit::testing;

namespace metkit::mars2grib::test {

//----------------------------------------------------------------------------------------------------------------------

class Setter : public eckit::LocalConfiguration, public KeySetter {
public:
    using eckit::LocalConfiguration::set;
    
    void setValue(const std::string& key, const std::string& value) override {
        set(key, value);
    }
    
    void setValue(const std::string& key, long value) override {
        set(key, value);
    }
    
    void setValue(const std::string& key, double value) override {
        set(key, value);
    }
};




//----------------------------------------------------------------------------------------------------------------------

CASE("test pdt selection") {
    {
        eckit::ValueMap initial{};
        Setter out;
        
        convertMars2Grib(initial, out);
        eckit::Log::info() << "Mapped: " << initial << " to " << out << std::endl;
        EXPECT(out.has("productDefinitionTemplateNumber"));
        EXPECT_EQUAL(out.getLong("productDefinitionTemplateNumber"), 0);
    }
    
    {
        eckit::ValueMap initial{{"paramId", 8}};
        Setter out;

        convertMars2Grib(initial, out);
        eckit::Log::info() << "Mapped: " << initial << " to " << out << std::endl;
        EXPECT(out.has("productDefinitionTemplateNumber"));
        EXPECT_EQUAL(out.getLong("productDefinitionTemplateNumber"), 8);
    }
    
    

}





//----------------------------------------------------------------------------------------------------------------------

}  // namespace test

int main(int argc, char** argv) {
    return run_tests(argc, argv);
}
