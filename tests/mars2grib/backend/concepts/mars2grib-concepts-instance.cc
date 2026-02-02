#include <iostream>
#include <string>

#include <eckit/config/LocalConfiguration.h>

#include <metkit/codes/api/CodesAPI.h>
#include "metkit/mars2grib/backend/concepts/concept_registry.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"

int main() {
    using namespace metkit::mars2grib::backend::cnpts;
    using Registry = ConceptRegistry<eckit::LocalConfiguration, eckit::LocalConfiguration, eckit::LocalConfiguration,
                                     eckit::LocalConfiguration, metkit::codes::CodesHandle>;

    Registry registry =
        concept_registry_instance<eckit::LocalConfiguration, eckit::LocalConfiguration, eckit::LocalConfiguration,
                                  eckit::LocalConfiguration, metkit::codes::CodesHandle>();

    eckit::LocalConfiguration aa;
    eckit::LocalConfiguration bb;
    eckit::LocalConfiguration cc;
    eckit::LocalConfiguration dd;
    auto pee = metkit::codes::codesHandleFromSample("GRIB2");


    // Loop su tutti i typeOfLevel e stampa la tabella
    for (const auto& [key, table] : registry.map) {
        const auto& [conceptName, conceptKind] = key;
        std::cout << "================================================================================================="
                     "================================"
                  << std::endl;
        std::cout << "Concept: type:" << conceptName << ", variant:" << conceptKind << "\n";

        for (int stage = 0; stage < NUM_STAGES; ++stage) {
            for (int sec = 0; sec < NUM_SECTIONS; ++sec) {
                bool assigned = (table[stage][sec] != nullptr);
                std::cout << "  stage " << stage << ", section " << sec << ": " << (assigned ? "yes" : "no")
                          << std::endl;
                if (assigned)
                    table[stage][sec](aa, bb, cc, dd, *pee);  // chiamata dummy
            }
        }
        std::cout << std::endl;
    }

    std::cout << "Map size: " << registry.map.size() << std::endl;

    return 0;
}
