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
/// @file detail/StatType.h
/// @brief Shared `stattype` parser used by both `ProductTime` and
///        `typeOfStatisticalProcessing`.
///
/// This header is **shared infrastructure**, not a deduction. It exposes:
///   - the `ParsedStatTypeBlock` value type (per-block, pre-mapped to GRIB
///     table values);
///   - the `parse_StatType_or_throw` function, which decodes the locked
///     `stattype` grammar (§7.7) into a vector of blocks in MARS textual
///     order (outermost → innermost).
///
/// **Why shared**: both `ProductTime` (which needs the *period* part) and
/// `typeOfStatisticalProcessing` (which needs the *operation* part) consume
/// the same `stattype` string. A single shared parser eliminates the drift
/// risk between them — both deductions size their output arrays in lock-step
/// from the same parse result.
///
/// **Allow-lists enforced by this parser** (parser-level hard errors):
///   - period unit ∈ `{mo, da}`           → §10.16 / §10.18 (a)
///   - operation  ∈ `{av, mn, mx, sd}`    → §10.16
///   - block ordering: `mo` precedes `da` → §10.17
///   - block count ∈ `{1, 2}`             → §10.16
///
/// The extended `Second`-inclusive allow-list applies only to the assembled
/// `ProductTime::statisticalWindows` array (where the innermost window may
/// originate from `timespan`); that check lives in
/// `makeProductTime_or_throw`.
///
/// See `deductions/timeProducts.md` §22 for the full normative
/// specification.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

// System includes
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

// eckit
#include "eckit/exception/Exceptions.h"

// Project
#include "metkit/config/LibMetkit.h"
#include "metkit/mars2grib/backend/deductions/detail/StatisticalWindow.h"
#include "metkit/mars2grib/backend/tables/timeUnits.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

namespace metkit::mars2grib::backend::deductions::detail {

// =============================================================
// 1. ParsedStatTypeBlock (§22.2)
// =============================================================

///
/// @brief One decoded `stattype` block, pre-mapped to GRIB table values.
///
/// - `timeWindow` carries the **period** part of the block (the
///   stattype-grammar pair `(period_unit, count)`). For locked grammar
///   tokens `mo` / `da`, the parser emits `count = 1` and
///   `unit = tables::TimeUnit::Month` / `tables::TimeUnit::Day`.
/// - `typeOfStatisticalProcessing` carries the **operation** part of the
///   block, already mapped to a value of GRIB Code Table 4.10:
///     - `av` → `Average`
///     - `mn` → `Minimum`
///     - `mx` → `Maximum`
///     - `sd` → `StandardDeviation`
///
struct ParsedStatTypeBlock {
    StatisticalWindow                            timeWindow;
    tables::TypeOfStatisticalProcessing          typeOfStatisticalProcessing;
};

// =============================================================
// 2. Single-token decoders (private helpers)
// =============================================================

namespace impl {

///
/// @brief Decode a single period token to a `StatisticalWindow`
///        with `count = 1`.
///
/// @throws Mars2GribDeductionException on unknown token (§10.16) or
///         narrow-allow-list violation (§10.18 (a)).
///
inline StatisticalWindow decodePeriod_or_throw(std::string_view s,
                                               const std::string& fullStatType) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (s == "da")
        return StatisticalWindow{tables::TimeUnit::Day, 1};
    if (s == "mo")
        return StatisticalWindow{tables::TimeUnit::Month, 1};

    throw Mars2GribDeductionException(
        "Invalid stattype period token [§10.16/§10.18(a)]: actual='"
            + std::string(s) + "', expected={'da','mo'} (in stattype='"
            + fullStatType + "')",
        Here());
}

///
/// @brief Decode a single operation token to a
///        `tables::TypeOfStatisticalProcessing`.
///
/// @throws Mars2GribDeductionException on unknown token (§10.16).
///
inline tables::TypeOfStatisticalProcessing
decodeOp_or_throw(std::string_view s, const std::string& fullStatType) {
    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (s == "av")
        return tables::TypeOfStatisticalProcessing::Average;
    if (s == "mn")
        return tables::TypeOfStatisticalProcessing::Minimum;
    if (s == "mx")
        return tables::TypeOfStatisticalProcessing::Maximum;
    if (s == "sd")
        return tables::TypeOfStatisticalProcessing::StandardDeviation;

    throw Mars2GribDeductionException(
        "Invalid stattype operation token [§10.16]: actual='"
            + std::string(s) + "', expected={'av','mn','mx','sd'} (in stattype='"
            + fullStatType + "')",
        Here());
}

}  // namespace impl

// =============================================================
// 3. parse_StatType_or_throw (§22.3)
// =============================================================

///
/// @brief Parse a `stattype` string per the locked grammar (§7.7).
///
/// Grammar:
/// @code
/// stattype  := block ('_' block)*
/// block     := period operation       (4 chars)
/// period    := 'mo' | 'da'
/// operation := 'av' | 'mn' | 'mx' | 'sd'
/// @endcode
///
/// Semantic constraints (parser-level hard errors):
///   - block count MUST be in `{1, 2}` (§10.16);
///   - if both `mo` and `da` blocks are present, `mo` MUST precede `da`
///     (§10.17);
///   - all unit and operation tokens MUST belong to the locked allow-lists
///     (§10.16, §10.18 (a)).
///
/// @param[in] stattype  Raw textual value of the MARS `stattype` keyword.
///                      MUST be non-empty; callers MUST NOT invoke the
///                      parser when the `stattype` key is absent from the
///                      MARS dictionary.
///
/// @return A vector of size 1 or 2, in MARS textual order (outermost first,
///         innermost last).
///
/// @throws Mars2GribDeductionException on any grammar (§10.16),
///         ordering (§10.17), or narrow-allow-list (§10.18 (a)) violation.
///
/// @note Each block's `typeOfStatisticalProcessing` is already mapped to a
///       value of GRIB Code Table 4.10; consumers do not need to perform
///       any further mapping.
///
inline std::vector<ParsedStatTypeBlock>
parse_StatType_or_throw(const std::string& stattype) {

    using metkit::mars2grib::utils::exceptions::Mars2GribDeductionException;

    if (stattype.empty()) {
        throw Mars2GribDeductionException(
            "Invalid stattype: empty string (caller MUST NOT invoke "
            "parse_StatType_or_throw when 'stattype' is absent)",
            Here());
    }

    std::vector<ParsedStatTypeBlock> blocks;

    std::size_t pos = 0;
    while (pos < stattype.size()) {

        if (pos + 4 > stattype.size()) {
            throw Mars2GribDeductionException(
                "Invalid stattype format [§10.16]: incomplete 4-char block "
                "at position " + std::to_string(pos) + " in '" + stattype + "'",
                Here());
        }

        StatisticalWindow window =
            impl::decodePeriod_or_throw(stattype.substr(pos, 2), stattype);
        tables::TypeOfStatisticalProcessing op =
            impl::decodeOp_or_throw(stattype.substr(pos + 2, 2), stattype);

        blocks.push_back(ParsedStatTypeBlock{window, op});

        pos += 4;
        if (pos < stattype.size()) {
            if (stattype[pos] != '_') {
                throw Mars2GribDeductionException(
                    "Invalid stattype separator [§10.16] at position "
                        + std::to_string(pos) + " in '" + stattype
                        + "': expected '_'",
                    Here());
            }
            ++pos;
        }
    }

    // Block-count limit: 1 or 2 blocks (§22.4 / §10.16).
    if (blocks.empty() || blocks.size() > 2) {
        throw Mars2GribDeductionException(
            "Invalid stattype [§10.16]: block count (" + std::to_string(blocks.size())
                + ") outside allowed range {1, 2} (in stattype='" + stattype + "')",
            Here());
    }

    // Semantic validation (§10.17): at most one mo, at most one da, mo precedes da.
    int moIndex = -1;
    int daIndex = -1;
    for (std::size_t i = 0; i < blocks.size(); ++i) {
        if (blocks[i].timeWindow.unit == tables::TimeUnit::Month) {
            if (moIndex != -1) {
                throw Mars2GribDeductionException(
                    "Invalid stattype [§10.17] '" + stattype
                        + "': more than one 'mo' block",
                    Here());
            }
            moIndex = static_cast<int>(i);
        }
        if (blocks[i].timeWindow.unit == tables::TimeUnit::Day) {
            if (daIndex != -1) {
                throw Mars2GribDeductionException(
                    "Invalid stattype [§10.17] '" + stattype
                        + "': more than one 'da' block",
                    Here());
            }
            daIndex = static_cast<int>(i);
        }
    }

    if (moIndex != -1 && daIndex != -1 && moIndex > daIndex) {
        throw Mars2GribDeductionException(
            "Invalid stattype [§10.17] '" + stattype
                + "': blocks not in outermost-to-innermost order "
                  "('mo' must precede 'da')",
            Here());
    }

    return blocks;
}

}  // namespace metkit::mars2grib::backend::deductions::detail
