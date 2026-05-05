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
/// @file typeOfStatisticalProcessing.h
/// @brief Public deduction header for `typeOfStatisticalProcessing`.
///
/// Exposes `resolve_TypeOfStatisticalProcessing_or_throw`, which produces a
/// `std::vector<tables::TypeOfStatisticalProcessing>` describing the
/// statistical processing applied at each time loop of a statistical
/// product, in outer→inner order.
///
/// **Self-contained**: this deduction does NOT depend on `ProductTime`.
/// It depends only on:
///   - `detail/StatType.h` — the shared `stattype` parser (§22)
///   - `tables/typeOfStatisticalProcessing.h` — GRIB Code Table 4.10 enum
///
/// **Implicit invariant** (well-documented; not enforced at runtime):
/// the output `std::vector` has length and order **identical** to
/// `ProductTime::statisticalWindows` resolved from the same MARS input —
/// outermost loop at index 0, innermost loop at index `size() - 1`.
/// Callers MUST rely on this invariant when zipping the two vectors.
///
/// **Precondition discharged by the type system**: the function takes a
/// `tables::TypeOfStatisticalProcessing` argument; a caller cannot
/// construct this argument without having already identified the product as
/// statistical (i.e., having decoded the inner statistical operation from
/// `paramId`). Therefore this deduction is structurally unreachable for
/// instant products (§9.1); no runtime check is performed for that case.
///
/// See `deductions/timeProducts.md` §23 for the full normative
/// specification.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

// System includes
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

// eckit
#include "eckit/exception/Exceptions.h"

// Project utilities
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"
#include "metkit/mars2grib/utils/generalUtils.h"
#include "metkit/mars2grib/utils/logUtils.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

// Detail (shared parser)
#include "metkit/mars2grib/backend/deductions/detail/StatType.h"

// Tables
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"

namespace metkit::mars2grib::backend::deductions {

///
/// @brief Resolve the per-loop GRIB `typeOfStatisticalProcessing` array for
///        one MARS statistical product.
///
/// @section Deduction contract
///   - Reads (MARS): `stattype`, `timespan` (presence only — for case
///                   classification per §23.5)
///   - Reads (par):  none
///   - Reads (opt):  none (signature-only, reserved)
///   - Writes:       none
///   - Side effects: one `MARS2GRIB_LOG_RESOLVE` line on success
///   - Failure mode: throws `Mars2GribDeductionException` (nested-with)
///
/// @section Output sizing per case (§23.5)
///
/// Local classification of MARS into one of the §9 cases (no `ProductTime`
/// dependency):
///
/// | MARS state                                              | §9 case | Output size | Output content |
/// |---------------------------------------------------------|---------|------------:|---------------------------------------------------------------------------------|
/// | `stattype` absent, `timespan` is a Duration             | §9.2    | 1           |
/// `[innerTypeOfStatisticalProcessing]`                                            | | `stattype` has N blocks,
/// `timespan` is a Duration       | §9.3    | N + 1       | `[op(block_0), …, op(block_{N-1}),
/// innerTypeOfStatisticalProcessing]`           | | `stattype` has 1 block,  `timespan = "none"`            | §9.4    |
/// 1           | `[innerTypeOfStatisticalProcessing]` (after equality assertion, §23.6)          | | `stattype` absent,
/// `timespan` absent              | §9.1    | unreachable | (precondition violated, see §23.4 — no defensive check) |
///
/// The innermost slot is **always** filled by the
/// `innerTypeOfStatisticalProcessing` argument — never by a parsed block.
///
/// @section fakeDoubleLoop equality assertion (§23.6)
///
/// In the §9.4 case, the single parsed `stattype` block carries a
/// redundantly encoded operation. This deduction asserts:
///
///     parsedBlocks[0].typeOfStatisticalProcessing
///         == innerTypeOfStatisticalProcessing
///
/// Disagreement is hard error §10.12.
///
/// @section Outer/inner asymmetry (§23.7)
///
///   - **Outer slots** (indices 0..size()-2 in the §9.3 case) are
///     constrained to `{Average, Minimum, Maximum, StandardDeviation}` by
///     the parser-level op allow-list (§22.5).
///   - **Innermost slot** is **unrestricted** within GRIB Code Table 4.10;
///     it can be any value the caller passes.
///
/// @tparam MarsDict_t   MARS dictionary type.
/// @tparam ParDict_t    Parameter dictionary type (currently unused).
/// @tparam OptDict_t    Options dictionary type (currently unused).
///
/// @param[in] innerTypeOfStatisticalProcessing
///            The pre-resolved inner statistical processing type, derived by
///            the caller from `paramId`.
/// @param[in] mars
///            MARS dictionary providing `stattype` and `timespan`.
/// @param[in] par
///            Parameter dictionary (signature-only).
/// @param[in] opt
///            Options dictionary (signature-only).
///
/// @return A `std::vector<tables::TypeOfStatisticalProcessing>` whose length
///         and order match `ProductTime::statisticalWindows` for the same
///         MARS input (implicit invariant, §23.3).
///
/// @throws metkit::mars2grib::utils::exceptions::Mars2GribDeductionException
///         on:
///           - §10.12 — fakeDoubleLoop equality violation;
///           - §10.16, §10.17, §10.18 (a) — propagated from
///             `parse_StatType_or_throw` (§22.7).
///         All failures are wrapped via `std::throw_with_nested`.
///
/// @note This function does NOT classify `timespan` exhaustively against the
///       §9 case table; it only distinguishes the three presence-states
///       (absent, Duration, "none") needed for §23.5 sizing. Misclassification
///       at this level (e.g. inputs that would fail §10.7 at the `ProductTime`
///       resolver) is undefined behavior here; callers are expected to invoke
///       this deduction only on inputs that also resolve cleanly through
///       `resolve_ProductTime_or_throw`.
///
template <class MarsDict_t, class ParDict_t, class OptDict_t>
std::vector<tables::TypeOfStatisticalProcessing> resolve_TypeOfStatisticalProcessing_or_throw(
    tables::TypeOfStatisticalProcessing innerTypeOfStatisticalProcessing, const MarsDict_t& mars, const ParDict_t& par,
    const OptDict_t& opt) {

    using metkit::mars2grib::utils::dict_traits::get_opt;
    using metkit::mars2grib::utils::dict_traits::get_or_throw;
    using metkit::mars2grib::utils::dict_traits::has;
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    // Suppress "unused parameter" warnings while preserving the documented
    // signature (par and opt are reserved for future use).
    (void)par;
    (void)opt;

    try {

        // =========================================================
        // Local case classification (§23.5)
        //
        // We look at presence of `stattype` and the textual value of
        // `timespan` to distinguish §9.2 / §9.3 / §9.4. Full §9-case
        // dispatch (with hard errors §10.6 / §10.7 / §10.8) remains the
        // responsibility of the `ProductTime` resolver; this deduction
        // only needs enough classification to size its output.
        // =========================================================

        const bool hasStatType = has(mars, "stattype");

        // Classify timespan: missing | "none" | Duration.
        // We do NOT parse the duration value here; only its presence and
        // the literal "none" matter for §23.5 sizing.
        bool hasTimespanNone     = false;
        bool hasTimespanDuration = false;
        if (has(mars, "timespan")) {
            std::optional<std::string> tsStr = get_opt<std::string>(mars, "timespan");
            if (tsStr.has_value()) {
                if (tsStr.value() == "none") {
                    hasTimespanNone = true;
                }
                else {
                    hasTimespanDuration = true;
                }
            }
            else {
                // Numeric-only timespan: a Duration per §7.6.
                hasTimespanDuration = true;
            }
        }

        // =========================================================
        // Parse `stattype` once (shared parser, §22).
        // =========================================================

        std::vector<detail::ParsedStatTypeBlock> blocks;
        if (hasStatType) {
            const std::string statTypeVal = get_or_throw<std::string>(mars, "stattype");
            blocks                        = detail::parse_StatType_or_throw(statTypeVal);
        }

        // =========================================================
        // Output assembly per §23.5.
        //
        // Default-fallback path: when MARS does not match any of the
        // statistical cases (§9.2 / §9.3 / §9.4) we still emit a size-1
        // vector containing the inner type. Other case-table violations
        // (e.g. §10.6 / §10.7 / §10.8) are not raised here; they are
        // raised by the `ProductTime` resolver when the caller invokes it
        // on the same MARS input (per the contract in §23.8 and the
        // class-level note above).
        // =========================================================

        std::vector<tables::TypeOfStatisticalProcessing> out;

        if (!hasStatType && hasTimespanDuration) {
            // ----- §9.2: Old-style single-loop -----
            out.reserve(1);
            out.push_back(innerTypeOfStatisticalProcessing);
        }
        else if (hasStatType && hasTimespanDuration) {
            // ----- §9.3: Old-style multi-loop -----
            out.reserve(blocks.size() + 1);
            for (const detail::ParsedStatTypeBlock& b : blocks) {
                out.push_back(b.typeOfStatisticalProcessing);
            }
            out.push_back(innerTypeOfStatisticalProcessing);
        }
        else if (hasStatType && hasTimespanNone) {
            // ----- §9.4: New-style fakeDoubleLoop -----
            //
            // Equality assertion (§23.6 / §10.12). Block count is
            // structurally guaranteed to be 1 here under the spec; if the
            // caller has supplied input that would fail §10.8 at the
            // `ProductTime` resolver (more than one block), the assertion
            // below acts on blocks[0] only — the multi-block error itself
            // is reported by the `ProductTime` resolver.
            if (blocks.empty()) {
                // §10.7: timespan='none' with no stattype block.
                // Defensive only — the parser rejects empty `stattype`,
                // and `hasStatType` is true here, so blocks.size() >= 1.
                throw Mars2GribDeductionException(
                    "typeOfStatisticalProcessing invariant violated [§10.7]: "
                    "timespan='none' requires exactly one stattype block, got 0",
                    Here());
            }

            const tables::TypeOfStatisticalProcessing parsedOp = blocks[0].typeOfStatisticalProcessing;

            if (parsedOp != innerTypeOfStatisticalProcessing) {
                throw Mars2GribDeductionException(
                    std::string("typeOfStatisticalProcessing invariant violated "
                                "[§10.12]: fakeDoubleLoop disagreement: parsed "
                                "stattype block operation ('") +
                        tables::enum2name_TypeOfStatisticalProcessing_or_throw(parsedOp) +
                        "') != innerTypeOfStatisticalProcessing argument ('" +
                        tables::enum2name_TypeOfStatisticalProcessing_or_throw(innerTypeOfStatisticalProcessing) + "')",
                    Here());
            }

            out.reserve(1);
            out.push_back(innerTypeOfStatisticalProcessing);
        }
        else {
            // Fallback: any other MARS shape (instant, or shapes that
            // would fail at the `ProductTime` resolver). Per §23.4 the
            // function is not expected to be called on such inputs;
            // produce a size-1 vector with the inner type so the caller
            // sees a defined result if they invoke us defensively.
            //
            // Strict §9-case enforcement is the `ProductTime` resolver's
            // responsibility; we deliberately do not duplicate §10.6 /
            // §10.7 / §10.8 here.
            out.reserve(1);
            out.push_back(innerTypeOfStatisticalProcessing);
        }

        // =========================================================
        // §23.10: composite RESOLVE log line (success path only)
        // =========================================================

        MARS2GRIB_LOG_RESOLVE([&]() {
            std::string msg = "`typeOfStatisticalProcessing` resolved from input dictionaries: ";
            msg += "innerTypeOfStatisticalProcessing='" +
                   tables::enum2name_TypeOfStatisticalProcessing_or_throw(innerTypeOfStatisticalProcessing) + "'";
            msg += " size='" + std::to_string(out.size()) + "'";
            msg += " typesOfStatisticalProcessing=[";
            for (std::size_t i = 0; i < out.size(); ++i) {
                if (i) {
                    msg += ",";
                }
                msg += "'" + tables::enum2name_TypeOfStatisticalProcessing_or_throw(out[i]) + "'";
            }
            msg += "]";
            return msg;
        }());

        return out;
    }
    catch (...) {

        // §11 / §23.9: nested rethrow with context.
        std::throw_with_nested(Mars2GribDeductionException("Unable to resolve typeOfStatisticalProcessing", Here()));
    }

    // Remove compiler warning
    mars2gribUnreachable();
}

}  // namespace metkit::mars2grib::backend::deductions
