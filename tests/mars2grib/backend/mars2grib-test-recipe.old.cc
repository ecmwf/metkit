#include <iostream>
#include <sstream>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/concepts/Recipes.h"
#include "metkit/mars2grib/backend/concepts/Section4Recipes.h"
#include "metkit/mars2grib/backend/concepts/capabilitiesRegistry.h"


int main() {

    using metkit::mars2grib::backend::concepts_::Registry;
    using metkit::mars2grib::backend::concepts_::Section4Recipes;


    // Start testing recipes
    std::cout << "Entries Test01: " << Section4Recipes::combination_count << std::endl;
    std::cout << "Entries Test02: " << Section4Recipes::xxx.entries.size() << std::endl;

    std::size_t cnt = 0;
    for (const auto& x : Section4Recipes::xxx.entries) {
        // std::cout << cnt++ << ", " << int(x.templateNumber) << ", " << int(x.indexCount) << std::endl;
        std::ostringstream json;
        json << "{ \"index\":" << cnt++ << ", \"template\":" << int(x.templateNumber) << ", \"concepts\":[";
        for (std::size_t i = 0; i < std::size_t(x.indexCount); i++) {
            std::string name;
            std::string variant;
            size_t j = std::size_t(x.indices[i]);
            name.clear();
            variant.clear();

            name    = std::string(Registry.conceptNames[j]);
            variant = std::string(Registry.variantNames[j]);
            if (i > 0) {
                json << ", ";
            }
            json << "\"" << name << "::" << variant << "\"";
        }
        json << "] }";

        std::cout << json.str() << std::endl;
        json.str("");  // clear the buffer
        json.clear();  // reset error/state flags
    }

    std::cout << "test03: " << Section4Recipes::xxx.globalMask << std::endl;
    std::cout << "test04: " << Section4Recipes::xxx.globalMask.count() << std::endl;

    // Exit point
    return 0;
};
