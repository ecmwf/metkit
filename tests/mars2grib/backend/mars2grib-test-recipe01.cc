#include <iostream>
#include <string>
#include <utility>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"


#include "metkit/mars2grib/backend/concepts/test.h"
#include "metkit/mars2grib/frontend-new/resolution/section-recipes/Recipe.h"
#include "metkit/mars2grib/frontend-new/resolution/section-recipes/Select.h"

using metkit::mars2grib::backend::concepts_::AnalysisType;
using metkit::mars2grib::backend::concepts_::DerivedConcept;
using metkit::mars2grib::backend::concepts_::Entry;
using metkit::mars2grib::backend::concepts_::GeneratingProcessConcept;
using metkit::mars2grib::backend::concepts_::LevelConcept;
using metkit::mars2grib::backend::concepts_::LevelType;
using metkit::mars2grib::backend::concepts_::ParamConcept;
using metkit::mars2grib::backend::concepts_::PointInTimeConcept;
using metkit::mars2grib::backend::concepts_::R;
using metkit::mars2grib::backend::concepts_::Recipe;
using metkit::mars2grib::backend::concepts_::Select;
using metkit::mars2grib::backend::concepts_::StatisticsConcept;
using metkit::mars2grib::backend::concepts_::WaveType;


template <typename RN, std::size_t I>
void printRecipe() {
    std::cout << " - Test" << I << "1: id=" << I + 1 << " of " << RN::count << std::endl;
    std::cout << " - Test" << I << "2: count=" << RN::count << std::endl;
    std::cout << " - Test" << I << "3: NSlots=" << RN::NSlots << std::endl;


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
    printAllRecipesImpl<RN>(std::make_index_sequence<RN::count>{});
}


int main() {

    using S4_R0 = Recipe<0, Select<GeneratingProcessConcept>, Select<PointInTimeConcept>, Select<LevelConcept>,
                         Select<ParamConcept>>;

    using S4_R12 = Recipe<12, Select<GeneratingProcessConcept>, Select<StatisticsConcept>, Select<LevelConcept>,
                          Select<ParamConcept>>;

    printAllRecipes<S4_R0>();
    // printAllRecipes<S4_R12>();

    for (const auto& e : S4_R12::entries) {
        std::cout << e.NSlots << ", " << e.TemplateNumber << std::endl;
    }

    // Exit point
    return 0;
};
