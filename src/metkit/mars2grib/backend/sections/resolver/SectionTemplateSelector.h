/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file SectionTemplateSelector.h
/// @brief Resolver responsible for selecting the template number of a GRIB section.
///
/// This header defines `SectionTemplateSelector`, the **central algorithmic
/// component** of the section resolver subsystem.
///
/// A `SectionTemplateSelector`:
/// - Is constructed once per section from declarative recipe definitions
/// - Precomputes all data structures required for efficient lookup
/// - Selects, at runtime, the correct section template number given the
/// active concept state
///
/// The selector is immutable after construction and optimized for hot-path
/// usage during encoding.
///
/// @ingroup mars2grib_backend_section_resolver
///
#pragma once

// System includes
#include <algorithm>
#include <array>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ActiveConceptsData.h"
#include "metkit/mars2grib/backend/sections/resolver/CompressionMask.h"
#include "metkit/mars2grib/backend/sections/resolver/Recipes.h"
#include "metkit/mars2grib/backend/sections/resolver/SectionLayoutData.h"
#include "metkit/mars2grib/backend/sections/resolver/TemplateSignatureKey.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::resolver {

// SectionTemplateSelector
///
/// @brief Section-local resolver for GRIB template selection.
///
/// `SectionTemplateSelector` encapsulates all logic required to select
/// the appropriate GRIB template number for a **single section**, given
/// the runtime active concept state.
///
/// The selector operates in two phases:
///
/// - **Construction phase (offline / once per section)**
/// - Expand declarative recipes into a flat payload
/// - Build a section-specific compression mask
/// - Precompute lookup indices
/// - Choose the optimal search strategy
///
/// - **Resolution phase (runtime / hot path)**
/// - Build a signature key from active concepts
/// - Apply section-specific compression
/// - Perform lookup using the preselected strategy
/// - Produce a `SectionLayoutData`
///
/// After construction, the selector is fully immutable.
///
class SectionTemplateSelector {
public:

    /// Runtime representation of active concepts
    using ActiveConcepts = ActiveConceptsData;

    /// Section-scoped recipe configuration
    using Recipes = metkit::mars2grib::backend::sections::resolver::dsl::Recipes;

    /// Section-scoped compression mask
    using CompressionMask = metkit::mars2grib::backend::sections::resolver::detail::CompressionMask;

    /// Expanded recipe entry produced by recipe expansion
    using ResolvedTemplateData = metkit::mars2grib::backend::sections::resolver::dsl::ResolvedTemplateData;

    /// Signature key type used for lookup
    using TemplateSignatureKey = metkit::mars2grib::backend::sections::resolver::detail::TemplateSignatureKey;

    /// Hash functor for signature keys
    using TemplateSignatureKeyHash = metkit::mars2grib::backend::sections::resolver::detail::TemplateSignatureKeyHash;

    ///
    /// @brief Select the section layout corresponding to the active concept state.
    ///
    /// @param[in] active Runtime active concept data
    ///
    /// @return Resolved section layout
    ///
    /// @throws Mars2GribGenericException
    /// If no matching template can be found
    ///
    const SectionLayoutData select_or_throw(const ActiveConcepts& active) const {
        using metkit::mars2grib::backend::sections::resolver::detail::make_SectionLayoutData_or_throw;
        const std::size_t id = searchFn_(*this, active);
        return make_SectionLayoutData_or_throw(sectionNumber_, payloads_[id]);
    }

    ///
    /// @brief Construct a selector from section recipes.
    ///
    /// This is the **only construction entry point**.
    ///
    /// @param[in] recipes Declarative recipe definitions for a section
    ///
    /// @return Fully constructed, immutable selector
    ///
    /// @throws Mars2GribGenericException
    /// If recipe expansion or index construction fails
    ///
    static SectionTemplateSelector make(const Recipes& recipes) {

        using metkit::mars2grib::backend::sections::resolver::detail::make_CompressionMask_or_throw;
        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

        // --------------------------------------------------------------------
        // 1. Expand recipes → resolved payload
        // --------------------------------------------------------------------
        std::vector<ResolvedTemplateData> payload = recipes.getPayload();

        if (payload.empty()) {
            throw Mars2GribGenericException("SectionTemplateSelector: empty payload", Here());
        }

        // --------------------------------------------------------------------
        // 2. Build section-specific compression mask
        // --------------------------------------------------------------------
        CompressionMask compressionMask = make_CompressionMask_or_throw(payload);

        // --------------------------------------------------------------------
        // 3. Build (compressedKey, payloadIndex) lookup index
        // --------------------------------------------------------------------
        std::vector<std::pair<TemplateSignatureKey, std::size_t>> index;
        index.reserve(payload.size());

        for (std::size_t i = 0; i < payload.size(); ++i) {
            const auto& entry = payload[i];

            TemplateSignatureKey globalKey{};
            globalKey.size = entry.count;

            for (std::size_t k = 0; k < entry.count; ++k) {
                globalKey.data[k] = entry.variantIndices[k];
            }

            TemplateSignatureKey compressedKey = compressionMask.compressKey(globalKey);

            index.emplace_back(std::move(compressedKey), i);
        }

        std::sort(index.begin(), index.end(), [](const auto& a, const auto& b) { return a.first < b.first; });

        // --------------------------------------------------------------------
        // 4. Reorder payload according to sorted index
        // --------------------------------------------------------------------
        std::vector<ResolvedTemplateData> orderedPayload;
        orderedPayload.reserve(index.size());

        for (const auto& [key, payloadIdx] : index) {
            orderedPayload.emplace_back(payload[payloadIdx]);
        }

        // --------------------------------------------------------------------
        // 5. Choose lookup strategy
        // --------------------------------------------------------------------
        const std::size_t N = index.size();

        Index indexVariant;
        SearchFn searchFn = nullptr;

        if (N == 1) {

            indexVariant = SingleIndex{index.front().first, 0};
            searchFn     = &SectionTemplateSelector::search_single;
        }
        else if (N < 16) {

            ArrayIndex vec;
            vec.reserve(N);

            for (std::size_t i = 0; i < N; ++i) {
                vec.emplace_back(index[i].first, i);
            }

            indexVariant = std::move(vec);
            searchFn     = &SectionTemplateSelector::search_linear;
        }
        else if (N < 256) {

            ArrayIndex vec;
            vec.reserve(N);

            for (std::size_t i = 0; i < N; ++i) {
                vec.emplace_back(index[i].first, i);
            }

            indexVariant = std::move(vec);
            searchFn     = &SectionTemplateSelector::search_binary;
        }
        else {

            HashIndex map;
            map.reserve(N);

            for (std::size_t i = 0; i < N; ++i) {
                map.emplace(index[i].first, i);
            }

            indexVariant = std::move(map);
            searchFn     = &SectionTemplateSelector::search_hash;
        }

        // --------------------------------------------------------------------
        // 6. Construct immutable selector
        // --------------------------------------------------------------------
        return SectionTemplateSelector{recipes.sectionId(), std::move(compressionMask), std::move(orderedPayload),
                                       std::move(indexVariant), searchFn};
    }

private:

    ///
    /// @brief Function pointer implementing the lookup strategy.
    ///
    /// This pointer selects the concrete search algorithm used at runtime
    /// to resolve a compressed signature key into a payload index.
    ///
    /// The function is chosen once during construction based on the
    /// number of template combinations and never changes afterward.
    ///
    /// All search functions:
    /// - Are pure
    /// - Are stateless
    /// - Throw if no matching template is found
    ///
    using SearchFn = std::size_t (*)(const SectionTemplateSelector&, const ActiveConcepts&);

    ///
    /// @brief Index optimized for a single template.
    ///
    /// Stores exactly one `(key, payloadIndex)` pair.
    /// Used when the section admits only one possible template.
    ///
    using SingleIndex = std::pair<TemplateSignatureKey, std::size_t>;

    ///
    /// @brief Linear / binary searchable index.
    ///
    /// Stores `(key, payloadIndex)` pairs in sorted order.
    /// Used for small to medium template spaces.
    ///
    using ArrayIndex = std::vector<SingleIndex>;

    ///
    /// @brief Hash-based index for large template spaces.
    ///
    /// Used when the number of admissible templates exceeds
    /// the threshold for efficient array-based search.
    ///
    using HashIndex = std::unordered_map<TemplateSignatureKey, std::size_t, TemplateSignatureKeyHash>;

    ///
    /// @brief Variant type holding the concrete index representation.
    ///
    /// Exactly one alternative is active at runtime, selected during
    /// construction and never changed afterward.
    ///
    using Index = std::variant<SingleIndex, ArrayIndex, HashIndex>;

    ///
    /// @brief GRIB section number handled by this selector.
    ///
    /// This value is immutable and propagated into the resulting
    /// `SectionLayoutData`.
    ///
    const std::size_t sectionNumber_;

    ///
    /// @brief Section-specific compression mask.
    ///
    /// Used to normalize signature keys before lookup by removing
    /// variants that never participate in this section.
    ///
    /// The mask is computed once during construction and reused
    /// for every lookup.
    ///
    const CompressionMask compressionMask_;

    ///
    /// @brief Ordered payload entries corresponding to resolved templates.
    ///
    /// The payload vector is reordered during construction so that
    /// its indices match those stored in the lookup index.
    ///
    /// The order of variants inside each payload entry is preserved
    /// and later used during encoding.
    ///
    const std::vector<ResolvedTemplateData> payloads_;

    ///
    /// @brief Lookup index mapping compressed keys to payload indices.
    ///
    /// The concrete representation is stored in @ref index_ and
    /// interpreted by the selected search function.
    ///
    const Index index_;

    ///
    /// @brief Pointer to the active search function.
    ///
    /// This pointer is guaranteed to be non-null after construction.
    ///
    const SearchFn searchFn_;

    ///
    /// @brief Resolve template index using a single-entry index.
    ///
    /// This search strategy is used when the section admits exactly
    /// one possible template.
    ///
    /// The active concept state is compressed and compared directly
    /// against the single stored key.
    ///
    /// @param[in] self   Selector instance
    /// @param[in] active Runtime active concept data
    ///
    /// @return Index of the matching payload entry
    ///
    /// @throws Mars2GribGenericException
    /// If the compressed key does not match the stored key
    ///
    SectionTemplateSelector(std::size_t sectionNumber, CompressionMask&& mask,
                            std::vector<ResolvedTemplateData>&& payloads, Index&& index, SearchFn fn) :
        sectionNumber_(sectionNumber),
        compressionMask_(std::move(mask)),
        payloads_(std::move(payloads)),
        index_(std::move(index)),
        searchFn_(fn) {}

    ///
    /// @brief Resolve template index using a single-entry index.
    ///
    /// This search strategy is used when the section admits exactly
    /// one possible template.
    ///
    /// The active concept state is compressed and compared directly
    /// against the single stored key.
    ///
    /// @param[in] self   Selector instance
    /// @param[in] active Runtime active concept data
    ///
    /// @return Index of the matching payload entry
    ///
    /// @throws Mars2GribGenericException
    /// If the compressed key does not match the stored key
    ///
    static std::size_t search_single(const SectionTemplateSelector& self, const ActiveConcepts& active) {

        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

        const auto& [key, id]  = std::get<SingleIndex>(self.index_);
        TemplateSignatureKey k = self.compressionMask_.compressKey(make_key(active));

        if (k == key) {
            return id;
        }

        throw Mars2GribGenericException("No matching recipe", Here());
    }

    ///
    /// @brief Resolve template index using linear search.
    ///
    /// This strategy performs a linear scan over a small number of
    /// `(key, payloadIndex)` pairs.
    ///
    /// It is selected when the template space is small enough that
    /// linear search is faster than binary or hash-based lookup.
    ///
    /// @param[in] self   Selector instance
    /// @param[in] active Runtime active concept data
    ///
    /// @return Index of the matching payload entry
    ///
    /// @throws Mars2GribGenericException
    /// If no matching key is found
    ///
    static std::size_t search_linear(const SectionTemplateSelector& self, const ActiveConcepts& active) {

        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

        const auto& vec        = std::get<ArrayIndex>(self.index_);
        TemplateSignatureKey k = self.compressionMask_.compressKey(make_key(active));

        for (const auto& [kk, id] : vec) {
            if (kk == k) {
                return id;
            }
        }

        throw Mars2GribGenericException("No matching recipe", Here());
    }

    ///
    /// @brief Resolve template index using binary search.
    ///
    /// This strategy performs a binary search over a sorted array
    /// of `(key, payloadIndex)` pairs.
    ///
    /// It is selected for medium-sized template spaces where
    /// logarithmic lookup outperforms linear scanning.
    ///
    /// @param[in] self   Selector instance
    /// @param[in] active Runtime active concept data
    ///
    /// @return Index of the matching payload entry
    ///
    /// @throws Mars2GribGenericException
    /// If no matching key is found
    ///
    static std::size_t search_binary(const SectionTemplateSelector& self, const ActiveConcepts& active) {

        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

        const auto& vec        = std::get<ArrayIndex>(self.index_);
        TemplateSignatureKey k = self.compressionMask_.compressKey(make_key(active));

        auto it = std::lower_bound(vec.begin(), vec.end(), k,
                                   [](const auto& p, const TemplateSignatureKey& key) { return p.first < key; });

        if (it != vec.end() && it->first == k) {
            return it->second;
        }

        throw Mars2GribGenericException("No matching recipe", Here());
    }

    ///
    /// @brief Resolve template index using hash-based lookup.
    ///
    /// This strategy performs an average O(1) lookup using an
    /// unordered map keyed by compressed signature keys.
    ///
    /// It is selected for large template spaces where array-based
    /// search would be inefficient.
    ///
    /// @param[in] self   Selector instance
    /// @param[in] active Runtime active concept data
    ///
    /// @return Index of the matching payload entry
    ///
    /// @throws Mars2GribGenericException
    /// If no matching key is found
    ///
    static std::size_t search_hash(const SectionTemplateSelector& self, const ActiveConcepts& active) {

        using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

        const auto& map        = std::get<HashIndex>(self.index_);
        TemplateSignatureKey k = self.compressionMask_.compressKey(make_key(active));

        auto it = map.find(k);
        if (it != map.end()) {
            return it->second;
        }

        throw Mars2GribGenericException("No matching recipe", Here());
    }

    ///
    /// @brief Build a template signature key from active concept data.
    ///
    /// The key is constructed by iterating over the list of active
    /// concept identifiers and collecting the corresponding global
    /// variant identifiers.
    ///
    /// The resulting key:
    /// - Reflects the active semantic state
    /// - Preserves no ordering guarantees
    /// - Must be normalized using the section compression mask
    ///
    /// @param[in] active Runtime active concept data
    ///
    /// @return Uncompressed template signature key
    ///
    static TemplateSignatureKey make_key(const ActiveConcepts& active) {

        TemplateSignatureKey key{};
        key.size = 0;

        for (std::size_t i = 0; i < active.count; ++i) {
            const std::size_t conceptId = active.activeConceptsIndices[i];

            key.data[key.size++] = active.activeVariantIndices[conceptId];
        }

        return key;
    }
};

}  // namespace metkit::mars2grib::backend::sections::resolver
