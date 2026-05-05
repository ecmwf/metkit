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
/// @file detail/StatisticalWindow.h
/// @brief Shared `StatisticalWindow` type used by `ProductTime` and the
///        shared `stattype` parser.
///
/// `StatisticalWindow` represents one statistical window as a
/// `(unit, count)` pair, reusing the GRIB-aligned `tables::TimeUnit` enum
/// (GRIB2 Code Table 4.4) rather than introducing a parallel "kind" enum.
///
/// This header is **shared infrastructure**, not a deduction. It is consumed
/// by:
///   - `deductions/detail/ProductTime.h` (stores values inside `ProductTime`)
///   - `deductions/detail/StatType.h`     (the shared `stattype` parser)
///
/// The two-level allow-list enforcement is documented in
/// `deductions/timeProducts.md` §10.18:
///   - the parser (§22) enforces the narrow `stattype`-grammar allow-list
///     `{Day, Month}` at parse time;
///   - the factory `make_ProductTime_or_throw` (§detail/ProductTime.h)
///     enforces the extended assembled-window allow-list `{Second, Day,
///     Month}` after window assembly.
///
/// See `deductions/timeProducts.md` §3, §21 for the full normative
/// specification.
///
/// @ingroup mars2grib_backend_deductions
///

#pragma once

// Project
#include "metkit/mars2grib/backend/tables/timeUnits.h"

namespace metkit::mars2grib::backend::deductions::detail {

///
/// @brief One statistical window: a `tables::TimeUnit` + a count.
///
/// Reuses the GRIB-aligned `tables::TimeUnit` enum (GRIB2 Code Table 4.4)
/// rather than introducing a parallel "kind" enum.
///
/// **Allow-list (assembled `ProductTime::statisticalWindows` entry)**:
/// only `Second`, `Day`, `Month` are legal. Any other `TimeUnit` value (in
/// particular `Hour`, `Minute`, `Hours3/6/12`, `Year`, `Decade`, `Normal`,
/// `Century`, `Missing`) is rejected by the factory `make_ProductTime_or_throw`
/// (§10.18 (b)).
///
/// **Allow-list (parser-produced entry)**: only `Day`, `Month` are legal.
/// The narrower `stattype`-grammar allow-list is enforced inside
/// `parse_StatType_or_throw` (§10.18 (a)).
///
/// **Invariant**: `count > 0` for any window stored inside a `ProductTime`
/// (§3.2 / §10.11). The default-constructed value (`count == 0`) is legal at
/// the C++ language level but illegal as a `ProductTime` member.
///
struct StatisticalWindow {
    tables::TimeUnit unit{tables::TimeUnit::Second};
    long             count{0};
};

}  // namespace metkit::mars2grib::backend::deductions::detail
