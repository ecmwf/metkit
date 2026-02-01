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

    for (long i = 0; i < GeneralRegistry::NVariants; i++) {
        std::cout << "Concept(" << static_cast<std::size_t>(GeneralRegistry::conceptIdArr[i]) << ", "
                  << GeneralRegistry::variantIdArr[i] << ") -> " << GeneralRegistry::conceptNameArr[i]
                  << "::" << GeneralRegistry::variantNameArr[i] << std::endl;
    }

    auto xxx = GeneralRegistry::make_id_array_from_concept<LevelConcept>();
    for (const auto& x : xxx) {
        std::cout << x << ", ";
    }
    std::cout << std::endl;

    auto yyy = GeneralRegistry::make_id_array_from_variants<LevelConcept, LevelType::SeaIceLayer,
                                                            LevelType::HeightAboveGround>();
    for (const auto& x : yyy) {
        std::cout << x << ", ";
    }
    std::cout << std::endl;

    // Exit point
    return 0;
};
