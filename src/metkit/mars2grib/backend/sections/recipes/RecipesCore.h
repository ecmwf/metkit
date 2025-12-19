#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace metkit::mars2grib::backend::sections::recipes {

struct ConceptSpec {
    std::string_view name;
    std::string_view type;

    constexpr ConceptSpec(std::string_view n, std::string_view t = "default") : name(n), type(t) {}
};

struct SectionRecipe {
    uint16_t templateNumber;
    std::vector<ConceptSpec> concepts;
};

// Helper DSL
constexpr ConceptSpec C(std::string_view n, std::string_view t = "default") {
    return ConceptSpec{n, t};
}

}  // namespace metkit::mars2grib::backend::sections::recipes
