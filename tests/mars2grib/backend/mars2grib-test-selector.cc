#include <iostream>
#include <string>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/Select.h"

int main() {

    using metkit::mars2grib::backend::concepts_::DerivedConcept;
    using metkit::mars2grib::backend::concepts_::LevelConcept;
    using metkit::mars2grib::backend::concepts_::LevelType;
    using metkit::mars2grib::backend::concepts_::StatisticsConcept;
    using metkit::mars2grib::backend::sections::resolver::dsl::Select;


    Select<LevelConcept>::debug_print("TEST01", std::cout);
    Select<StatisticsConcept>::debug_print("TEST02", std::cout);
    Select<DerivedConcept>::debug_print("TEST03", std::cout);
    Select<LevelConcept, LevelType::Surface, LevelType::HeightAboveGround, LevelType::HeightAboveSeaAt10M>::debug_print(
        "TEST04", std::cout);

    // Exit point
    return 0;
};
