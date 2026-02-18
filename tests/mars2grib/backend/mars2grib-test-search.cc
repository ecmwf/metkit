/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

// dictionary access traits
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "eckit/log/Log.h"
#include "eckit/testing/Test.h"
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ActiveConceptsData.h"
#include "metkit/mars2grib/backend/sections/resolver/ResolvedTemplateData.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionLayoutData.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionTemplateSelector.h"
#include "metkit/mars2grib/frontend/resolution/section-recipes/SectionTemplateSelectors.h"

using ActiveConceptsData   = metkit::mars2grib::backend::sections::resolver::ActiveConceptsData;
using ResolvedTemplateData = metkit::mars2grib::backend::sections::resolver::dsl::ResolvedTemplateData;

std::tuple<ActiveConceptsData, std::size_t> make_ActiveConceptData_from_PayloadEntry(const ResolvedTemplateData& pe) {
    using metkit::mars2grib::backend::concepts_::GeneralRegistry;

    ActiveConceptsData res{};
    res.count = pe.count;

    for (std::size_t i = 0; i < res.activeConceptsIndices.size(); ++i) {
        res.activeConceptsIndices[i] = GeneralRegistry::missing;
    }

    for (std::size_t i = 0; i < res.activeVariantIndices.size(); ++i) {
        res.activeVariantIndices[i] = GeneralRegistry::missing;
    }

    for (std::size_t i = 0; i < res.count; ++i) {
        std::size_t vid               = pe.variantIndices[i];
        std::size_t cid               = GeneralRegistry::conceptIdArr[vid];
        res.activeConceptsIndices[i]  = cid;
        res.activeVariantIndices[cid] = vid;
    }

    return std::tuple{res, pe.templateNumber};
}


CASE("Section 0") {
    using metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section0Recipes;

    const auto payload  = Section0Recipes.getPayload();
    const auto selector = SectionTemplateSelector::make(Section0Recipes);
    for (std::size_t i = 0; i < payload.size(); ++i) {
        auto [activeConcepts, expectedTemplateNumber] = make_ActiveConceptData_from_PayloadEntry(payload[i]);
        const auto& sectionLayoutData                 = selector.select_or_throw(activeConcepts);
        eckit::Log::error() << "Section 0 :: " << i << " expected template number " << expectedTemplateNumber
                            << " but found " << sectionLayoutData.templateNumber << std::endl;
        ASSERT(expectedTemplateNumber == sectionLayoutData.templateNumber);
    }
}

CASE("Section 1") {
    using metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section1Recipes;

    const auto payload  = Section1Recipes.getPayload();
    const auto selector = SectionTemplateSelector::make(Section1Recipes);
    for (std::size_t i = 0; i < payload.size(); ++i) {
        auto [activeConcepts, expectedTemplateNumber] = make_ActiveConceptData_from_PayloadEntry(payload[i]);
        const auto& sectionLayoutData                 = selector.select_or_throw(activeConcepts);
        eckit::Log::error() << "Section 1 :: " << i << " expected template number " << expectedTemplateNumber
                            << " but found " << sectionLayoutData.templateNumber << std::endl;
        ASSERT(expectedTemplateNumber == sectionLayoutData.templateNumber);
    }
}

CASE("Section 2") {
    using metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section2Recipes;

    const auto payload  = Section2Recipes.getPayload();
    const auto selector = SectionTemplateSelector::make(Section2Recipes);
    for (std::size_t i = 0; i < payload.size(); ++i) {
        auto [activeConcepts, expectedTemplateNumber] = make_ActiveConceptData_from_PayloadEntry(payload[i]);
        const auto& sectionLayoutData                 = selector.select_or_throw(activeConcepts);
        eckit::Log::error() << "Section 2 :: " << i << " expected template number " << expectedTemplateNumber
                            << " but found " << sectionLayoutData.templateNumber << std::endl;
        ASSERT(expectedTemplateNumber == sectionLayoutData.templateNumber);
    }
}

CASE("Section 3") {
    using metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section3Recipes;

    const auto payload  = Section3Recipes.getPayload();
    const auto selector = SectionTemplateSelector::make(Section3Recipes);
    for (std::size_t i = 0; i < payload.size(); ++i) {
        auto [activeConcepts, expectedTemplateNumber] = make_ActiveConceptData_from_PayloadEntry(payload[i]);
        const auto& sectionLayoutData                 = selector.select_or_throw(activeConcepts);
        eckit::Log::error() << "Section 3 :: " << i << " expected template number " << expectedTemplateNumber
                            << " but found " << sectionLayoutData.templateNumber << std::endl;
        ASSERT(expectedTemplateNumber == sectionLayoutData.templateNumber);
    }
}

CASE("Section 4") {
    using metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section4Recipes;

    const auto payload  = Section4Recipes.getPayload();
    const auto selector = SectionTemplateSelector::make(Section4Recipes);
    for (std::size_t i = 0; i < payload.size(); ++i) {
        auto [activeConcepts, expectedTemplateNumber] = make_ActiveConceptData_from_PayloadEntry(payload[i]);
        const auto& sectionLayoutData                 = selector.select_or_throw(activeConcepts);
        eckit::Log::error() << "Section 4 :: " << i << " expected template number " << expectedTemplateNumber
                            << " but found " << sectionLayoutData.templateNumber << std::endl;
        ASSERT(expectedTemplateNumber == sectionLayoutData.templateNumber);
    }
}

CASE("Section 5") {
    using metkit::mars2grib::backend::sections::resolver::SectionLayoutData;
    using metkit::mars2grib::backend::sections::resolver::SectionTemplateSelector;
    using metkit::mars2grib::frontend::resolution::recipes::impl::Section5Recipes;

    const auto payload  = Section5Recipes.getPayload();
    const auto selector = SectionTemplateSelector::make(Section5Recipes);
    for (std::size_t i = 0; i < payload.size(); ++i) {
        auto [activeConcepts, expectedTemplateNumber] = make_ActiveConceptData_from_PayloadEntry(payload[i]);
        const auto& sectionLayoutData                 = selector.select_or_throw(activeConcepts);
        eckit::Log::error() << "Section 5 :: " << i << " expected template number " << expectedTemplateNumber
                            << " but found " << sectionLayoutData.templateNumber << std::endl;
        ASSERT(expectedTemplateNumber == sectionLayoutData.templateNumber);
    }
}

int main(int argc, char** argv) {
    return eckit::testing::run_tests(argc, argv);
}
