/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/**
 * @file tablesOp.h
 * @brief Implementation of the GRIB tables-versioning concept (`tables`).
 *
 * This header defines the applicability rules and execution logic for the
 * **tables concept** within the mars2grib backend.
 *
 * The concept is responsible for selecting and encoding the GRIB2 tables
 * versions used to interpret code tables and definitions in the produced
 * message:
 * - `tablesVersion`
 * - `localTablesVersion`
 *
 * The concept is executed during the **allocation stage** for the
 * *Identification Section* (Section 1). The rationale is that table versions
 * influence how subsequent keys may be interpreted by ecCodes and downstream
 * tools, and therefore should be established early and deterministically.
 *
 * Two variants are supported:
 * - `TablesType::Default`: use the latest tables version provided by ecCodes.
 * - `TablesType::Custom`: override the tables version from the parametrization
 *   dictionary.
 *
 * All failures are wrapped in a `Mars2GribConceptException` providing full
 * concept context (concept name, variant, stage, section).
 *
 * @note
 * The namespace name `concepts_` is intentionally used instead of `concepts`
 * to avoid potential conflicts with the C++20 `concepts` language feature.
 *
 * @ingroup mars2grib_backend_concepts
 */
#pragma once

// System includes
#include <string>

// dictionary traits
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

// Core concept includes
#include "metkit/mars2grib/backend/concepts/conceptCore.h"
#include "metkit/mars2grib/backend/concepts/tables/tablesEnum.h"

// Deductions
#include "metkit/mars2grib/backend/deductions/localTablesVersion.h"
#include "metkit/mars2grib/backend/deductions/tablesVersion.h"

// Utils
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2grib-exception.h"

namespace metkit::mars2grib::backend::concepts_ {

/**
 * @brief Compile-time applicability predicate for the `tables` concept.
 *
 * This predicate determines whether the `tables` concept is applicable for
 * a given combination of:
 * - encoding stage
 * - GRIB section
 * - concept variant
 *
 * The default applicability rule enables this concept only when:
 * - `Stage == StageAllocate`
 * - `Section == SecIdentificationSection`
 *
 * @tparam Stage   Encoding stage (compile-time constant)
 * @tparam Section GRIB section index (compile-time constant)
 * @tparam Variant Tables concept variant
 *
 * @return `true` if applicable, `false` otherwise.
 */
template <std::size_t Stage, std::size_t Section, TablesType Variant>
constexpr bool tablesApplicable() {

    // Conditions to apply concept
    return ((Stage == StageAllocate) && (Section == SecIdentificationSection));
}


/**
 * @brief Execute the `tables` concept operation.
 *
 * This function sets GRIB table versioning keys in the output dictionary:
 * - `tablesVersion`
 * - `localTablesVersion`
 *
 * The logic is variant-dependent:
 *
 * ### Variant `TablesType::Default`
 * - `tablesVersion` is derived from an ecCodes GRIB2 sample via
 *   `resolve_TablesVersionLatest_or_throw()`.
 * - `localTablesVersion` is deduced via
 *   `resolve_LocalTablesVersion_or_throw()` (currently returns `0`).
 *
 * ### Variant `TablesType::Custom`
 * - `tablesVersion` is taken from the parametrization dictionary (override)
 *   via `resolve_TablesVersionCustom_or_throw()`.
 * - `localTablesVersion` is deduced via
 *   `resolve_LocalTablesVersion_or_throw()`.
 *
 * The concept is intended to be deterministic: either it explicitly sets both
 * keys, or it fails with an exception.
 *
 * @tparam Stage      Encoding stage (compile-time constant)
 * @tparam Section    GRIB section index (compile-time constant)
 * @tparam Variant    Tables concept variant
 * @tparam MarsDict_t Type of the MARS input dictionary
 * @tparam ParDict_t  Type of the parameter dictionary
 * @tparam OptDict_t  Type of the options dictionary
 * @tparam OutDict_t  Type of the GRIB output dictionary
 *
 * @param[in]  mars MARS input dictionary (unused by the current implementation)
 * @param[in]  par  Parameter dictionary (used for custom tables override)
 * @param[in]  opt  Options dictionary
 * @param[out] out  Output GRIB dictionary to be populated
 *
 * @throws metkit::mars2grib::utils::exceptions::Mars2GribConceptException
 *         If:
 *         - the concept is invoked outside its applicability domain
 *         - any table version deduction fails
 *         - any output encoding step fails
 *
 * @note
 * The `tablesVersion` key may affect downstream decoding and validation;
 * therefore, it should be set before any logic that depends on specific
 * table entries or local definitions.
 *
 * @warning
 * In the provided implementation, the call:
 * `resolve_TablesVersionCustom_or_throw(par, par, opt)`
 * appears inconsistent with the signature previously shown for that deduction
 * (expected `(mars, par, opt)`). If the intention is to read only from `par`,
 * consider either:
 * - adjusting the deduction signature, or
 * - passing the correct `mars` dictionary as first argument.
 *
 * @see tablesApplicable
 * @see deductions::resolve_TablesVersionLatest_or_throw
 * @see deductions::resolve_TablesVersionCustom_or_throw
 * @see deductions::resolve_LocalTablesVersion_or_throw
 */
template <std::size_t Stage, std::size_t Section, TablesType Variant, class MarsDict_t, class ParDict_t,
          class OptDict_t, class OutDict_t>
void TablesOp(const MarsDict_t& mars, const ParDict_t& par, const OptDict_t& opt, OutDict_t& out) {

    using metkit::mars2grib::utils::dict_traits::set_or_throw;
    using metkit::mars2grib::utils::exceptions::Mars2GribConceptException;

    if constexpr (tablesApplicable<Stage, Section, Variant>()) {

        try {

            MARS2GRIB_LOG_CONCEPT(tables);

            // Global deductions
            long localTablesVersionVal = deductions::resolve_LocalTablesVersion_or_throw(mars, par, opt);

            // set in output dictionary
            if constexpr (Variant == TablesType::Custom) {

                // Deductions
                long tablesVersionVal = deductions::resolve_TablesVersionCustom_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "tablesVersion", tablesVersionVal);
                set_or_throw<long>(out, "localTablesVersion", localTablesVersionVal);
            }
            else if constexpr (Variant == TablesType::Default) {

                // Deductions
                long tablesVersionVal = deductions::resolve_TablesVersionLatest_or_throw(mars, par, opt);

                // Encoding
                set_or_throw<long>(out, "tablesVersion", tablesVersionVal);
                set_or_throw<long>(out, "localTablesVersion", localTablesVersionVal);
            }
            else {
                MARS2GRIB_CONCEPT_THROW(tables, "Unsupported variant for `tables` concept...");
            }
        }
        catch (...) {
            MARS2GRIB_CONCEPT_RETHROW(tables, "Unable to set `tables` concept...");
        }

        // Successful operation
        return;
    }

    // Concept invoked outside its applicability domain
    MARS2GRIB_CONCEPT_THROW(tables, "Concept called when not applicable...");

    // Remove compiler warning
    __builtin_unreachable();
}

}  // namespace metkit::mars2grib::backend::concepts_
