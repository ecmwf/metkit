#pragma once

#include <array>

#include "metkit/mars2grib/backend/sections/resolver/SectionTemplateSelector.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/impl/section0Recipes.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/impl/section1Recipes.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/impl/section2Recipes.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/impl/section3Recipes.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/impl/section4Recipes.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/impl/section5Recipes.h"
#include "metkit/mars2grib/utils/generalUtils.h"

namespace metkit::mars2grib::frontend::resolution::recipes {


struct SectionTemplateSelectors {

    using GeneralRegistry         = metkit::mars2grib::backend::concepts_::GeneralRegistry;
    using SectionTemplateSelector = metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;

    static inline const std::array<SectionTemplateSelector, GeneralRegistry::NSections> value = [] {
        return std::array<SectionTemplateSelector, GeneralRegistry::NSections>{
            SectionTemplateSelector::make(impl::Section0Recipes), SectionTemplateSelector::make(impl::Section1Recipes),
            SectionTemplateSelector::make(impl::Section2Recipes), SectionTemplateSelector::make(impl::Section3Recipes),
            SectionTemplateSelector::make(impl::Section4Recipes), SectionTemplateSelector::make(impl::Section5Recipes)};
    }();
};


}  // namespace metkit::mars2grib::frontend::resolution::recipes