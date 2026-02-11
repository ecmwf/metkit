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
/// @file CompressionMask.h
/// @brief Section-specific mask used to normalize template signature keys.
///
/// This header defines `CompressionMask`, an **internal, immutable semantic
/// object** used by the section resolver to *filter and normalize*
/// `TemplateSignatureKey` instances prior to lookup.
///
/// --------------------------------------------------------------------------
///
/// @section compressionmask_what What is a compression mask?
///
/// A compression mask is a **section-specific filter** applied to a
/// `TemplateSignatureKey` in order to remove concept variants that are
/// *irrelevant* for a given GRIB section.
///
/// Not all concepts participate in all sections. If irrelevant variants
/// were left in the key, every comparison against section-specific template
/// definitions would fail, and the combinatorial space would explode.
///
/// For example:
/// - When resolving **Section 4**, concepts that never participate in
/// Section 4 must be ignored.
/// - Otherwise, template lookup would implicitly require considering
/// combinations across *different sections*, which is semantically wrong.
///
/// The compression mask ensures that only the variants that *can actually
/// appear* in a given section contribute to key comparison.
///
/// --------------------------------------------------------------------------
///
/// @section compressionmask_why Why is it needed?
///
/// A `TemplateSignatureKey` represents the **full active concept state**.
/// However:
///
/// - Section template definitions are **section-local**
/// - Key comparison must therefore be **section-local**
///
/// The compression mask:
/// - Removes variants that never appear in any recipe for the section
/// - Normalizes keys so that lookup depends only on relevant information
///
/// This step is **mandatory** for correct and efficient section resolution.
///
/// --------------------------------------------------------------------------
///
/// @section compressionmask_how How is it computed?
///
/// The compression mask is computed *once per section* from the section
/// recipe payload, using a two-phase process.
///
/// @subsection compressionmask_phase1 Phase 1: variant collection
///
/// All resolved template entries for the section are scanned, and the set
/// of **all variant identifiers that ever participate** in the section is
/// collected.
///
/// Variants that never appear in any recipe entry are marked as invalid.
///
/// @subsection compressionmask_phase2 Phase 2: index assignment
///
/// The collected variants are assigned **dense compressed indices**.
///
/// This step finalizes the mask by mapping:
/// - irrelevant variants → `invalid`
/// - relevant variants   → dense indices `[0, compressedSize)`
///
/// The resulting mask is immutable and section-specific.
///
/// --------------------------------------------------------------------------
///
/// @section compressionmask_sorting Order normalization vs encoding order
///
/// A crucial design point is the distinction between:
///
/// - **Variant identity** (used for template lookup)
/// - **Variant order**    (used for encoding)
///
/// The compression mask supports two compression modes:
///
/// - `compressKey`
/// Produces a **sorted** compressed key.
/// Sorting ensures that template lookup depends *only on which variants
/// are present*, not on their order.
///
/// - `compressUnsortedKey`
/// Produces an **order-preserving** compressed key.
/// This is useful when order must be retained.
///
/// Importantly:
/// - **Order is intentionally ignored during lookup**
/// - **Order is still preserved in the payload**
///
/// Encoding relies on the ordered variant list stored in
/// `ResolvedTemplateData`, not on the compressed key.
///
/// This asymmetry is deliberate and fundamental to the resolver design.
///
/// --------------------------------------------------------------------------
///
/// @section compressionmask_relationship Relationship with Select and Recipes
///
/// - `Select` defines:
/// - Which variants are admissible
/// - The order in which concepts must be encoded
///
/// - `Recipe` and `ResolvedTemplateData` preserve this order
///
/// - `CompressionMask`:
/// - Ignores order
/// - Retains only variant identity
/// - Operates strictly at the lookup level
///
/// Together, these components allow:
/// - Declarative, ordered encoding
/// - Order-independent, efficient template matching
///
/// @note
/// This type is an internal implementation detail of the resolver and is not
/// part of the public API.
///
/// @ingroup mars2grib_backend_section_resolver
///
#pragma once

// System includes
#include <array>
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

// Project includes
#include "metkit/mars2grib/backend/concepts/GeneralRegistry.h"
#include "metkit/mars2grib/backend/sections/resolver/ResolvedTemplateData.h"
#include "metkit/mars2grib/backend/sections/resolver/TemplateSignatureKey.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::sections::resolver::detail {

///
/// @brief Section-specific compression mask for template signature keys.
///
/// `CompressionMask` is an immutable object that filters and normalizes
/// `TemplateSignatureKey` instances so that they can be compared and
/// matched against section-local template definitions.
///
/// The mask is:
/// - Computed once per section
/// - Derived solely from the section recipe payload
/// - Independent of the active concept state
///
class CompressionMask {
public:

    /// Registry providing global variant identifiers
    using GeneralRegistry = metkit::mars2grib::backend::concepts_::GeneralRegistry;

    /// Sentinel value marking invalid / irrelevant variants
    static constexpr std::size_t invalid = GeneralRegistry::invalid;

    ///
    /// @brief Number of variants retained after compression.
    ///
    std::size_t compressedSize() const noexcept { return compressedSize_; }

    ///
    /// @brief Compress a signature key while preserving variant order.
    ///
    /// Irrelevant variants are removed, but the relative order of the
    /// remaining variants is preserved.
    ///
    /// @param[in] in Input signature key
    ///
    /// @return Compressed key with preserved order
    ///
    TemplateSignatureKey compressUnsortedKey(const TemplateSignatureKey& in) const noexcept {

        TemplateSignatureKey out{};
        out.size = 0;

        for (std::size_t i = 0; i < in.size; ++i) {
            const std::size_t v = in.data[i];
            if (mask_[v] != invalid) {
                out.data[out.size++] = v;
            }
        }

        return out;
    }

    ///
    /// @brief Compress a signature key and normalize its order.
    ///
    /// Irrelevant variants are removed and the remaining variants are
    /// inserted into the output key in sorted order.
    ///
    /// This guarantees that key comparison depends only on *which variants
    /// are present*, not on their order.
    ///
    /// @param[in] in Input signature key
    ///
    /// @return Sorted, compressed key
    ///
    TemplateSignatureKey compressKey(const TemplateSignatureKey& in) const noexcept {

        TemplateSignatureKey out{};
        out.size = 0;

        for (std::size_t i = 0; i < in.size; ++i) {
            const std::size_t v = in.data[i];

            if (mask_[v] == invalid) {
                continue;
            }

            std::size_t j = out.size;
            while (j > 0 && out.data[j - 1] > v) {
                out.data[j] = out.data[j - 1];
                --j;
            }

            out.data[j] = v;
            ++out.size;
        }

        return out;
    }

    ///
    /// @brief Print a human-readable description of the compression mask.
    ///
    /// @param[in]  prefix Line prefix used for indentation
    /// @param[out] os     Output stream
    ///
    void debug_print(const std::string& prefix, std::ostream& os) const {

        using metkit::mars2grib::backend::concepts_::GeneralRegistry;

        os << prefix << " :: Compressed size: " << compressedSize_ << "\n";
        os << prefix << " :: Compression mask indices: [ ";

        for (std::size_t v = 0; v < mask_.size(); ++v) {
            os << mask_[v];
            if (v + 1 < mask_.size()) {
                os << ", ";
            }
        }

        os << " ]\n";

        os << prefix << " :: Compression mask names: [ ";

        for (std::size_t v = 0; v < mask_.size(); ++v) {
            std::size_t id = mask_[v];
            if (id != GeneralRegistry::invalid) {
                std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
                std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
                os << "\"" << cname << "::" << vname << "\"";
            }
            else {
                os << "\"invalid\"";
            }
            if (v + 1 < mask_.size()) {
                os << ", ";
            }
        }

        os << " ]" << std::endl;
    }

    ///
    /// @brief Convert the compression mask to a JSON-like string.
    ///
    /// This method produces a diagnostic, human-readable representation of the
    /// compression mask. The output is intended exclusively for debugging and
    /// introspection and is not guaranteed to be valid JSON.
    ///
    /// The representation exposes:
    /// - the compressed size
    /// - the full variant-to-compressed-index mapping
    ///
    /// Variants mapped to `invalid` explicitly indicate concepts that never
    /// participate in the section and are therefore removed during key compression.
    ///
    /// @return JSON-style string describing the compression mask
    ///
    std::string debug_to_json() const {

        using metkit::mars2grib::backend::concepts_::GeneralRegistry;

        std::ostringstream oss;

        oss << "{ \"CompressionMask\": { "
            << "\"compressedSize\": " << compressedSize_ << ", "
            << "\"maskIndices\": [ ";

        for (std::size_t v = 0; v < mask_.size(); ++v) {
            oss << mask_[v];
            if (v + 1 < mask_.size()) {
                oss << ", ";
            }
        }

        oss << " ], ";

        oss << "\"maskNames\": [ ";

        for (std::size_t v = 0; v < mask_.size(); ++v) {
            std::size_t id = mask_[v];
            if (id != GeneralRegistry::invalid) {
                std::string cname = std::string(GeneralRegistry::conceptNameArr[id]);
                std::string vname = std::string(GeneralRegistry::variantNameArr[id]);
                oss << "\"" << cname << "::" << vname << "\"";
            }
            else {
                oss << "\"invalid\"";
            }
            if (v + 1 < mask_.size()) {
                oss << ", ";
            }
        }

        oss << " ] } }";

        return oss.str();
    }

private:

    /// Mapping from global variant identifier to compressed index or invalid
    const std::array<std::size_t, GeneralRegistry::NVariants> mask_;

    /// Number of retained variants
    const std::size_t compressedSize_;

    CompressionMask(std::array<std::size_t, GeneralRegistry::NVariants>&& mask, std::size_t compressedSize) :
        mask_(std::move(mask)), compressedSize_(compressedSize) {}

    friend CompressionMask make_CompressionMask_or_throw(
        const std::vector<metkit::mars2grib::backend::sections::resolver::dsl::ResolvedTemplateData>&);
};

///
/// @brief Build a compression mask from section recipe payload.
///
/// This factory computes a section-specific compression mask by scanning
/// all resolved template entries and collecting the set of variants that
/// ever participate in the section.
///
/// @param[in] payload Resolved template payload for the section
///
/// @return Fully constructed compression mask
///
/// @throws Mars2GribGenericException
/// If the payload is empty or inconsistent
///
inline CompressionMask make_CompressionMask_or_throw(
    const std::vector<metkit::mars2grib::backend::sections::resolver::dsl::ResolvedTemplateData>& payload) {

    using metkit::mars2grib::backend::concepts_::GeneralRegistry;
    using metkit::mars2grib::utils::exceptions::Mars2GribGenericException;

    if (payload.empty()) {
        throw Mars2GribGenericException("CompressionMask: empty payload", Here());
    }

    std::array<std::size_t, GeneralRegistry::NVariants> mask{};
    mask.fill(0);

    for (const auto& entry : payload) {
        for (std::size_t i = 0; i < entry.count; ++i) {
            const std::size_t v = entry.variantIndices[i];
            if (v >= GeneralRegistry::NVariants) {
                throw Mars2GribGenericException("CompressionMask: variant index out of range", Here());
            }
            ++mask[v];
        }
    }

    std::size_t cnt = 0;
    for (std::size_t v = 0; v < GeneralRegistry::NVariants; ++v) {
        if (mask[v] == 0) {
            mask[v] = GeneralRegistry::invalid;
        }
        else {
            mask[v] = cnt++;
        }
    }

    return CompressionMask{std::move(mask), cnt};
}

}  // namespace metkit::mars2grib::backend::sections::resolver::detail
