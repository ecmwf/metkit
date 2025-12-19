#include <iostream>
#include <string>
#include <vector>

#include "eckit/testing/Test.h"

#include "metkit/mars2grib/backend/sections/recipes/Recipes.h"


using metkit::mars2grib::backend::sections::recipes::findRecipe;
using metkit::mars2grib::backend::sections::recipes::SectionRecipe;

// -------------------------------------------------------------------------------------------------

CASE("Recipes: lookup existing recipe") {

    const SectionRecipe* r = findRecipe(/*section*/ 4, /*template*/ 11);

    EXPECT(r != nullptr);
    EXPECT(r->templateNumber == 11);
    EXPECT(r->concepts.size() == 5);

    EXPECT(r->concepts[0].name == "generatingProcess");
    EXPECT(r->concepts[1].name == "statistics");
    EXPECT(r->concepts[2].name == "level");
    EXPECT(r->concepts[3].name == "param");
    EXPECT(r->concepts[4].name == "ensemble");

    EXPECT(r->concepts[4].type == "individual");
}

// -------------------------------------------------------------------------------------------------

CASE("Recipes: default type is explicit") {

    const SectionRecipe* r = findRecipe(4, 0);

    EXPECT(r != nullptr);
    EXPECT(r->concepts.size() == 4);

    for (const auto& c : r->concepts) {
        EXPECT(!c.type.empty());
        EXPECT(c.type == "default");
    }
}

// -------------------------------------------------------------------------------------------------

CASE("Recipes: order is preserved") {

    const SectionRecipe* r = findRecipe(3, 40);

    EXPECT(r != nullptr);
    EXPECT(r->concepts.size() == 2);

    EXPECT(r->concepts[0].name == "shapeOfTheEarth");
    EXPECT(r->concepts[1].name == "representation");

    EXPECT(r->concepts[1].type == "gaussian");
}

// -------------------------------------------------------------------------------------------------

CASE("Recipes: reforecast reference time") {

    const SectionRecipe* r = findRecipe(4, 60);

    EXPECT(r != nullptr);

    EXPECT(r->concepts[1].name == "referenceTime");
    EXPECT(r->concepts[1].type == "reforecast");
}

// -------------------------------------------------------------------------------------------------

CASE("Recipes: unknown template returns null") {

    const SectionRecipe* r = findRecipe(4, 9999);

    EXPECT(r == nullptr);
}

// -------------------------------------------------------------------------------------------------

CASE("Recipes: unknown section returns null") {

    const SectionRecipe* r = findRecipe(99, 0);

    EXPECT(r == nullptr);
}

// -------------------------------------------------------------------------------------------------

CASE("Recipes: section 5 packing") {

    const SectionRecipe* r = findRecipe(5, 42);

    EXPECT(r != nullptr);
    EXPECT(r->concepts.size() == 1);

    EXPECT(r->concepts[0].name == "packing");
    EXPECT(r->concepts[0].type == "ccsds");
}

// -------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
