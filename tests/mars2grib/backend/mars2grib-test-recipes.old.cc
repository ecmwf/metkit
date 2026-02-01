#include <iostream>
#include <string>
#include <utility>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"


#include "metkit/mars2grib/backend/concepts/Recipe.h"
#include "metkit/mars2grib/backend/concepts/Recipes.h"
#include "metkit/mars2grib/backend/concepts/Select.h"
#include "metkit/mars2grib/backend/concepts/test.h"

using metkit::mars2grib::backend::concepts_::AnalysisType;
using metkit::mars2grib::backend::concepts_::DerivedConcept;
using metkit::mars2grib::backend::concepts_::EnsembleConcept;
using metkit::mars2grib::backend::concepts_::EnsembleType;
using metkit::mars2grib::backend::concepts_::Entry;
using metkit::mars2grib::backend::concepts_::GeneratingProcessConcept;
using metkit::mars2grib::backend::concepts_::LevelConcept;
using metkit::mars2grib::backend::concepts_::LevelType;
using metkit::mars2grib::backend::concepts_::make_id_array_from_concept;
using metkit::mars2grib::backend::concepts_::make_id_array_from_variants;
using metkit::mars2grib::backend::concepts_::ParamConcept;
using metkit::mars2grib::backend::concepts_::PointInTimeConcept;
using metkit::mars2grib::backend::concepts_::R;
using metkit::mars2grib::backend::concepts_::Recipe;
using metkit::mars2grib::backend::concepts_::Recipes;
using metkit::mars2grib::backend::concepts_::Select;
using metkit::mars2grib::backend::concepts_::StatisticsConcept;
using metkit::mars2grib::backend::concepts_::WaveType;

template <typename RN, std::size_t I>
void printRecipe() {
    std::cout << " - Test" << I << "1: id=" << I + 1 << " of " << RN::countEntries << std::endl;
    std::cout << " - Test" << I << "2: Nentries=" << RN::countEntries << std::endl;
    std::cout << " - Test" << I << "3: Nrecipes=" << RN::countRecipes << std::endl;


    Entry<R::NConcepts> yyy = RN::template getEntry<R::NConcepts, I>();

    std::cout << " - Test" << I << "4: TemplateNumber=" << int(yyy.TemplateNumber) << std::endl;
    std::cout << " - Test" << I << "5: NSlots=" << int(yyy.NSlots) << std::endl;
    std::cout << " - Test" << I << "6: globalIndices=";
    std::string sep = "[ ";
    for (std::size_t i = 0; i < yyy.NSlots; ++i) {
        std::cout << sep << int(yyy.globalIndices_[i]);
        sep = ", ";
    }
    std::cout << " ]" << std::endl;


    std::cout << " - Test" << I << "7: globalNames=";
    sep = "[ ";
    for (std::size_t i = 0; i < yyy.NSlots; ++i) {
        int tmp = int(yyy.globalIndices_[i]);
        std::cout << sep << "\"" << R::conceptNameArr[tmp] << "::" << R::variantNameArr[tmp] << "\"";
        sep = ", ";
    }
    std::cout << " ]" << std::endl;
    std::cout << " -------------------------------------------------------------------------------------" << std::endl;
    std::cout << std::endl;
}

template <typename RN, std::size_t... Is>
void printAllRecipesImpl(std::index_sequence<Is...>) {
    (printRecipe<RN, Is>(), ...);
}

template <typename RN>
void printAllRecipes() {
    printAllRecipesImpl<RN>(std::make_index_sequence<RN::countEntries>{});
}


int main() {
    using S4_R0 = Recipe<0, Select<GeneratingProcessConcept>, Select<PointInTimeConcept>, Select<LevelConcept>,
                         Select<ParamConcept>>;

    using S4_R1 = Recipe<1, Select<GeneratingProcessConcept>, Select<PointInTimeConcept>, Select<LevelConcept>,
                         Select<ParamConcept>, Select<EnsembleConcept, EnsembleType::Individual>>;

    using S4_R2 = Recipe<2, Select<GeneratingProcessConcept>, Select<PointInTimeConcept>, Select<LevelConcept>,
                         Select<ParamConcept>, Select<DerivedConcept>>;

    using S4_R8 = Recipe<8, Select<GeneratingProcessConcept>, Select<StatisticsConcept>, Select<LevelConcept>,
                         Select<ParamConcept>>;

    using S4_R11 = Recipe<11, Select<GeneratingProcessConcept>, Select<StatisticsConcept>, Select<LevelConcept>,
                          Select<ParamConcept>, Select<EnsembleConcept, EnsembleType::Individual>>;

    using S4_R12 = Recipe<12, Select<GeneratingProcessConcept>, Select<StatisticsConcept>, Select<LevelConcept>,
                          Select<ParamConcept>, Select<DerivedConcept>>;

    using Section4Recipes = Recipes<S4_R0, S4_R1, S4_R2, S4_R8, S4_R11, S4_R12>;

    // using Section4Recipes = Recipes<S4_R0, S4_R1>;

    std::cout << " - Num. Recipes: " << Section4Recipes::countRecipes << std::endl;
    std::cout << " - Num. Entries: " << Section4Recipes::countEntries << std::endl;
    std::cout << S4_R0::count << std::endl;
    std::cout << S4_R1::count << std::endl;

    for (const auto& x : Section4Recipes::offsets) {
        std::cout << x << std::endl;
    }

    for (const auto& e : Section4Recipes::entries<R::NConcepts>) {
        std::cout << "HEllo World" << std::endl;
    }


    // printAllRecipes<Section4Recipes>();

    // Exit point
    return 0;
};
