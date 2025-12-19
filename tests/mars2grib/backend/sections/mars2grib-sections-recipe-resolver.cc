#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars2grib/backend/sections/sections_recipes.h"


int main(int argc, char** argv) {

    // Example usage
    uint16_t sectionId  = 4;
    uint16_t templateId = 43;

    const std::optional<metkit::mars2grib::backend::sections::ConceptList> concepts =
        metkit::mars2grib::backend::sections::resolveSectionTemplateConcepts(sectionId, templateId);
    if (concepts) {
        for (const auto& concept : *concepts) {
            if (concept.type) {
                printf("Concept: %.*s, Type: %.*s\n", (int)concept.name.size(), concept.name.data(),
                       (int)concept.type->size(), concept.type->data());
            }
            else {
                printf("Concept: %.*s, Type: null\n", (int)concept.name.size(), concept.name.data());
            }
        }
    }
    else {
        printf("No recipe found for Section %d, Template %d\n", sectionId, templateId);
    }

    return 0;
}
