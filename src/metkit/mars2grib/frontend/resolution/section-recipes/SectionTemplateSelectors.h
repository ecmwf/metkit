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
#include "metkit/mars2grib/utils/Profiling.h"

namespace metkit::mars2grib::frontend::resolution::recipes {


struct SectionTemplateSelectors {

    using GeneralRegistry         = metkit::mars2grib::backend::concepts_::GeneralRegistry;

    template <class Cntx_t>
    static const std::array<metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector<Cntx_t>,
                            GeneralRegistry::NSections>&
    get(Cntx_t& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        using SectionTemplateSelector =
            metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector<Cntx_t>;

        static const std::array<SectionTemplateSelector, GeneralRegistry::NSections> value{
            SectionTemplateSelector::make(impl::Section0Recipes, utils::profiling::callSite(cntx, Here())),
            SectionTemplateSelector::make(impl::Section1Recipes, utils::profiling::callSite(cntx, Here())),
            SectionTemplateSelector::make(impl::Section2Recipes, utils::profiling::callSite(cntx, Here())),
            SectionTemplateSelector::make(impl::Section3Recipes, utils::profiling::callSite(cntx, Here())),
            SectionTemplateSelector::make(impl::Section4Recipes, utils::profiling::callSite(cntx, Here())),
            SectionTemplateSelector::make(impl::Section5Recipes, utils::profiling::callSite(cntx, Here()))};
        const std::array<metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector<Cntx_t>,
                         GeneralRegistry::NSections>& result = value;
        utils::profiling::profileExitFunction(cntx, Here());
        return result;
    }
};


}  // namespace metkit::mars2grib::frontend::resolution::recipes
