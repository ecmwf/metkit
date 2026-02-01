#include <iostream>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/EncodingPlan.h"
#include "metkit/mars2grib/backend/concepts/capabilitiesRegistry.h"
#include "metkit/mars2grib/backend/concepts/conceptTypelist.h"

int main() {

    using metkit::mars2grib::backend::concepts_::AnalysisType;
    using metkit::mars2grib::backend::concepts_::id_v;
    using metkit::mars2grib::backend::concepts_::LevelType;
    using metkit::mars2grib::backend::concepts_::Registry;
    using metkit::mars2grib::backend::concepts_::totalVariants;
    using metkit::mars2grib::backend::concepts_::WaveType;

    std::cout << " - Analysis:      " << id_v<AnalysisType::Default> << std::endl;
    std::cout << " - Level:         " << id_v<LevelType::HeightAboveSea> << std::endl;
    std::cout << " - Wave:          " << id_v<WaveType::Spectra> << std::endl;
    std::cout << " - Wave:          " << id_v<WaveType::Period> << std::endl;
    std::cout << " - Wave:          " << id_v<WaveType::Default> << std::endl;
    std::cout << " - TotalVariants: " << totalVariants << std::endl;

    for (long i = 0; i < totalVariants; i++) {
        std::cout << "Concept(" << Registry.cid[i] << ", " << Registry.vid[i] << ") -> " << Registry.conceptNames[i]
                  << "::" << Registry.variantNames[i] << std::endl;
    }

    // Exit point
    return 0;
};
