#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/testing/Test.h"

#include "metkit/codes/api/CodesAPI.h"


// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// section registry
#include "metkit/mars2grib/backend/sections/initializers/section_registry.h"

int main(int argc, char** argv) {


    using metkit::mars2grib::backend::sections::initializers::Fn;
    using metkit::mars2grib::backend::sections::initializers::getSectionInitializerFn;

    const std::string yaml(R"json({
step: 12,
lat: 45.5,
flag: true,
name: test
})json");

    const eckit::YAMLConfiguration root(yaml);

    eckit::LocalConfiguration mars_dict(root);
    eckit::LocalConfiguration geo_dict(root);
    eckit::LocalConfiguration par_dict(root);
    eckit::LocalConfiguration opt_dict(root);

    auto handlePtr = metkit::codes::codesHandleFromSample("GRIB2");
    auto& out_dict = *handlePtr;

    auto Fcn = getSectionInitializerFn<eckit::LocalConfiguration, eckit::LocalConfiguration, eckit::LocalConfiguration,
                                       eckit::LocalConfiguration, metkit::codes::CodesHandle>(2, 15);

    Fcn(mars_dict, geo_dict, par_dict, opt_dict, out_dict);

    std::cout << "Test completed successfully." << std::endl;

    return 0;
}
