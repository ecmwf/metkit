#include <iostream>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"

int main() {

    using metkit::mars2grib::backend::concepts_::AnalysisType;
    using metkit::mars2grib::backend::concepts_::GeneralRegistry;
    using metkit::mars2grib::backend::concepts_::LevelConcept;
    using metkit::mars2grib::backend::concepts_::LevelType;
    using metkit::mars2grib::backend::concepts_::WaveType;

    std::cout << " - Analysis:      " << GeneralRegistry::globalIndex(AnalysisType::Default) << std::endl;
    std::cout << " - Level:         " << GeneralRegistry::globalIndex(LevelType::HeightAboveSea) << std::endl;
    std::cout << " - Wave:          " << GeneralRegistry::globalIndex(WaveType::Spectra) << std::endl;
    std::cout << " - Wave:          " << GeneralRegistry::globalIndex(WaveType::Period) << std::endl;
    std::cout << " - Wave:          " << GeneralRegistry::globalIndex(WaveType::Default) << std::endl;
    std::cout << " - Total number of Variants: " << GeneralRegistry::NVariants << std::endl;

    for (long i = 0; i < GeneralRegistry::NConcepts; i++) {

        std::cout << "Concept ID " << i << " - with range: [" << GeneralRegistry::conceptOffsets[i] << " to " << GeneralRegistry::conceptOffsets[i + 1] << "] -> has variants:" << std::endl;
        for (long j = GeneralRegistry::conceptOffsets[i];
             j < GeneralRegistry::conceptOffsets[i + 1]; j++) {

            std::cout << "Concept(" << i << ", "
                      << GeneralRegistry::variantIdArr[j] << ") -> " << GeneralRegistry::conceptNameArr[j]
                      << "::" << GeneralRegistry::variantNameArr[j] << std::endl;
        }
        std::cout << std::endl;
    }

    // Exit point
    return 0;
};
