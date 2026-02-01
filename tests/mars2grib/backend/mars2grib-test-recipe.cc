#include <iostream>
#include <string>
#include <utility>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"


#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/Recipe.h"
#include "metkit/mars2grib/backend/sections/resolver/Select.h"


int main() {

    using metkit::mars2grib::backend::concepts_::DerivedConcept;
    using metkit::mars2grib::backend::concepts_::GeneratingProcessConcept;
    using metkit::mars2grib::backend::concepts_::LevelConcept;
    using metkit::mars2grib::backend::concepts_::ParamConcept;
    using metkit::mars2grib::backend::concepts_::PointInTimeConcept;
    using metkit::mars2grib::backend::concepts_::StatisticsConcept;
    using metkit::mars2grib::backend::sections::resolver::dsl::make_recipe;
    using metkit::mars2grib::backend::sections::resolver::dsl::Recipe;
    using metkit::mars2grib::backend::sections::resolver::dsl::Select;


    const Recipe S4_R0 = make_recipe<0, Select<GeneratingProcessConcept>, Select<PointInTimeConcept>,
                                     Select<LevelConcept>, Select<ParamConcept> >();


    const Recipe S4_R12 = make_recipe<12, Select<GeneratingProcessConcept>, Select<StatisticsConcept>,
                                      Select<LevelConcept>, Select<ParamConcept>, Select<DerivedConcept> >();


    std::string jsonS4R0  = S4_R0.debug_to_json();
    std::string jsonS4R12 = S4_R12.debug_to_json();

    std::cout << jsonS4R0 << std::endl;
    std::cout << jsonS4R12 << std::endl;

    S4_R0.debug_print("TEST01", std::cout);
    S4_R12.debug_print("TEST02", std::cout);

    // Exit point
    return 0;
};
