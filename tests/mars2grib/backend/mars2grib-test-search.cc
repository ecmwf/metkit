#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <tuple>
#include <cassert>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ResolvedTemplateData.h"
#include "metkit/mars2grib/backend/sections/resolver/ActiveConceptsData.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionTemplateSelector.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/SectionTemplateSelectors.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionLayoutData.h"

using ActiveConceptsData = metkit::mars2grib::backend::sections::resolver::ActiveConceptsData;
using ResolvedTemplateData = metkit::mars2grib::backend::sections::resolver::dsl::ResolvedTemplateData;

std::tuple<ActiveConceptsData,std::size_t> make_ActiveConceptData_from_PayloadEntry(const ResolvedTemplateData& pe) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;

    ActiveConceptsData res{};

    res.count = pe.count;

    for ( std::size_t i=0; i<res.activeConceptsIndices.size(); ++i ){
        res.activeConceptsIndices[i] = GeneralRegistry::invalid;
    }

    for ( std::size_t i=0; i<res.activeVariantIndices.size(); ++i ){
        res.activeVariantIndices[i] = GeneralRegistry::invalid;
    }

    for (std::size_t i = 0; i < res.count; ++i) {
        std::size_t vid = pe.variantIndices[i];
        std::size_t cid = GeneralRegistry::conceptIdArr[vid];
        res.activeConceptsIndices[i] = cid;
        res.activeVariantIndices[cid] = vid;
    }

    return std::tuple{res, pe.templateNumber};
};

int main() {

    using metkit::mars2grib::backend::sections::resolver::detail::CompressionMask;
    using metkit::mars2grib::backend::sections::resolver::detail::make_CompressionMask_or_throw;
    using metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;
    using metkit::mars2grib::backend::sections::resolver::dsl::ResolvedTemplateData;
    using metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::debug::debug_print_SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::debug::debug_print_ActiveConceptsData;

    using metkit::mars2grib::frontend::resolution::recipes::impl::Section0Recipes;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section1Recipes;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section2Recipes;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section3Recipes;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section4Recipes;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section5Recipes;

    const auto payload0 = Section0Recipes.getPayload();
    const auto payload1 = Section1Recipes.getPayload();
    const auto payload2 = Section2Recipes.getPayload();
    const auto payload3 = Section3Recipes.getPayload();
    const auto payload4 = Section4Recipes.getPayload();
    const auto payload5 = Section5Recipes.getPayload();

    {
        const SectionTemplateSelector selector = SectionTemplateSelector::make(Section0Recipes);

        for ( std::size_t i=0; i<payload0.size(); ++i ){
            auto [ activeConcepts, expectedTemplateNumber ] = make_ActiveConceptData_from_PayloadEntry(payload0[i]);

            // debug_print_ActiveConceptsData( activeConcepts, "ACTIVE-DATA", std::cout );

            const SectionLayoutData& sectionLayoutData = selector.select_or_throw(activeConcepts);

            // debug_print_SectionLayoutData( sectionLayoutData, "SECTION-LAYOUT", std::cout );

            std::cout << "Section0 :: " << i << " - " << expectedTemplateNumber << " - " << sectionLayoutData.templateNumber << std::endl;
            assert( expectedTemplateNumber == sectionLayoutData.templateNumber );
        }
    }

    {
        const SectionTemplateSelector selector = SectionTemplateSelector::make(Section1Recipes);

        for ( std::size_t i=0; i<payload1.size(); ++i ){
            auto [ activeConcepts, expectedTemplateNumber ] = make_ActiveConceptData_from_PayloadEntry(payload1[i]);

            // debug_print_ActiveConceptsData( activeConcepts, "ACTIVE-DATA", std::cout );

            const SectionLayoutData& sectionLayoutData = selector.select_or_throw(activeConcepts);

            // debug_print_SectionLayoutData( sectionLayoutData, "SECTION-LAYOUT", std::cout );

            std::cout << "Section1 :: " << i << " - " << expectedTemplateNumber << " - " << sectionLayoutData.templateNumber << std::endl;
            assert( expectedTemplateNumber == sectionLayoutData.templateNumber );
        }
    }

    {
        const SectionTemplateSelector selector = SectionTemplateSelector::make(Section2Recipes);

        for ( std::size_t i=0; i<payload2.size(); ++i ){
            auto [ activeConcepts, expectedTemplateNumber ] = make_ActiveConceptData_from_PayloadEntry(payload2[i]);

            // debug_print_ActiveConceptsData( activeConcepts, "ACTIVE-DATA", std::cout );

            const SectionLayoutData& sectionLayoutData = selector.select_or_throw(activeConcepts);

            // debug_print_SectionLayoutData( sectionLayoutData, "SECTION-LAYOUT", std::cout );

            std::cout << "Section2 :: " << i << " - " << expectedTemplateNumber << " - " << sectionLayoutData.templateNumber << std::endl;
            assert( expectedTemplateNumber == sectionLayoutData.templateNumber );
        }
    }

    {
        const SectionTemplateSelector selector = SectionTemplateSelector::make(Section3Recipes);

        for ( std::size_t i=0; i<payload3.size(); ++i ){
            auto [ activeConcepts, expectedTemplateNumber ] = make_ActiveConceptData_from_PayloadEntry(payload3[i]);

            // debug_print_ActiveConceptsData( activeConcepts, "ACTIVE-DATA", std::cout );

            const SectionLayoutData& sectionLayoutData = selector.select_or_throw(activeConcepts);

            // debug_print_SectionLayoutData( sectionLayoutData, "SECTION-LAYOUT", std::cout );

            std::cout << "Section3 :: " << i << " - " << expectedTemplateNumber << " - " << sectionLayoutData.templateNumber << std::endl;
            assert( expectedTemplateNumber == sectionLayoutData.templateNumber );
        }
    }

    {
        const SectionTemplateSelector selector = SectionTemplateSelector::make(Section4Recipes);

        for ( std::size_t i=0; i<payload4.size(); ++i ){
            auto [ activeConcepts, expectedTemplateNumber ] = make_ActiveConceptData_from_PayloadEntry(payload4[i]);

            // debug_print_ActiveConceptsData( activeConcepts, "ACTIVE-DATA", std::cout );

            const SectionLayoutData& sectionLayoutData = selector.select_or_throw(activeConcepts);

            // debug_print_SectionLayoutData( sectionLayoutData, "SECTION-LAYOUT", std::cout );

            std::cout << "Section4 :: " << i << " - " << expectedTemplateNumber << " - " << sectionLayoutData.templateNumber << std::endl;
            assert( expectedTemplateNumber == sectionLayoutData.templateNumber );
        }
    }

    {
        const SectionTemplateSelector selector = SectionTemplateSelector::make(Section5Recipes);

        for ( std::size_t i=0; i<payload5.size(); ++i ){
            auto [ activeConcepts, expectedTemplateNumber ] = make_ActiveConceptData_from_PayloadEntry(payload5[i]);

            // debug_print_ActiveConceptsData( activeConcepts, "ACTIVE-DATA", std::cout );

            const SectionLayoutData& sectionLayoutData = selector.select_or_throw(activeConcepts);

            // debug_print_SectionLayoutData( sectionLayoutData, "SECTION-LAYOUT", std::cout );

            std::cout << "Section5 :: " << i << " - " << expectedTemplateNumber << " - " << sectionLayoutData.templateNumber << std::endl;
            assert( expectedTemplateNumber == sectionLayoutData.templateNumber );
        }
    }

    // Exit point
    return 0;
};
