#include <iostream>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"


#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/dsl.h"

#include "metkit/mars2grib/frontend/resolution/section-recipes/SectionTemplateSelectors.h"


int main() {

    using metkit::mars2grib::backend::sections::resolver::dsl::ResolvedTemplateData;
    using metkit::mars2grib::backend::sections::resolver::dsl::debug::debug_convert_ResolvedTemplateData_to_json;

    const auto payload0 = metkit::mars2grib::frontend::resolution::recipes::impl::Section0Recipes.getPayload();
    const auto payload1 = metkit::mars2grib::frontend::resolution::recipes::impl::Section1Recipes.getPayload();
    const auto payload2 = metkit::mars2grib::frontend::resolution::recipes::impl::Section2Recipes.getPayload();
    const auto payload3 = metkit::mars2grib::frontend::resolution::recipes::impl::Section3Recipes.getPayload();
    const auto payload4 = metkit::mars2grib::frontend::resolution::recipes::impl::Section4Recipes.getPayload();
    const auto payload5 = metkit::mars2grib::frontend::resolution::recipes::impl::Section5Recipes.getPayload();

    metkit::mars2grib::frontend::resolution::recipes::impl::Section3Recipes.debug_print("TEST01", std::cout);

    std::optional<std::string> sep = std::nullopt;
    std::cout << "{ \"Section4Recipes\":[" << std::endl;
    for (const auto& e : payload4) {
        if (!sep.has_value()) {
            std::cout << debug_convert_ResolvedTemplateData_to_json(e);
            sep = ", ";
        }
        else {
            std::cout << sep.value() << std::endl << debug_convert_ResolvedTemplateData_to_json(e);
        }
    }
    std::cout << "]}" << std::endl;

    // Exit point
    return 0;
};
