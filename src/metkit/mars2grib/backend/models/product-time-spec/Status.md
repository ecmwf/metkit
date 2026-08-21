<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# ProductTimeSpec Status

## Purpose

This file is the live iteration tracker for the rewrite described in
`Requirements.md` and constrained by `Constraints.md`.

It must be updated at each significant implementation step.

## Current State

- planning completed for the rewrite structure;
- temporary markdown replaced with `Requirements.md`, `Status.md`, and
  `Constraints.md`;
- early normalized-input refinement completed in code;
- raw domain artifact extension completed in code;
- anchor callback review and rewrite completed in code;
- domain callback review and rewrite completed in code;
- shape callback review and rewrite completed in code;
- anchor, domain, and shape local raw check callbacks implemented in code;
- shape registry checker dispatch implemented in code;
- normalization helper added in code but not wired yet;
- final cross-check stage not started yet.

## Completed Tasks

- `0.0` Extend normalized input with precomputed step-zero statistical-processing allowance.
- `0.1` Update `isAllowed_InnerTypeOfStatisticalProcessingAtStepZero` semantics.
- `0.2` Add precomputed step-zero allowance to `ProductTimeSpecInput`.
- `0.3` Compute the new input member during `make_ProductTimeSpecInput_or_throw`.
- `0.4` Update zero-step shape callbacks to consume the input member.
- `1.0` Extend raw domain artifact.
- `1.1` Review `match_Analysis_Domain`.
- `1.2` Review `build_Analysis_Domain`.
- `1.3` Review `match_Forecast_Domain`.
- `1.4` Review `build_Forecast_Domain`.
- `1.5` Review `match_FromStartForecast_Domain`.
- `1.6` Review `build_FromStartForecast_Domain`.
- `1.7` Review `match_SeasonalForecast_Domain`.
- `1.8` Review `build_SeasonalForecast_Domain`.
- `1.9` Review `match_SynopticAnalysis_Domain`.
- `1.10` Review `build_SynopticAnalysis_Domain`.
- `2.0` Add per-anchor check callbacks.
- `2.1` Review `match_ForecastAnalysis_Anchor`.
- `2.2` Review `build_ForecastAnalysis_Anchor`.
- `2.3` Review `match_Hindcast_Anchor`.
- `2.4` Review `build_Hindcast_Anchor`.
- `2.5` Review `match_SeasonalClimate_Anchor`.
- `2.6` Review `build_SeasonalClimate_Anchor`.
- `3.0` Add per-domain check callbacks.
- `4.0` Add per-shape check callbacks.
- `4.1` to `4.42` Review and refactor all shape callbacks.

## Agreed Decisions

- Keep existing absolute domain datetimes.
- Extend `ProductTimeSpecDomain` with:
  - `bool isSynoptic`
  - `long startOffsetHoursFromReference`
  - `long endOffsetHoursFromReference`
- Normalize after callbacks, not inside callbacks.
- Encoding needs final time ranges in hours.
- Final time increments need to be seconds or missing.
- Month-valued raw ranges must be converted using the real placed interval.
- Sub-monthly normalized hour validation source of truth is the language
  timespan support set.
- Monthly normalized hour validation source of truth is the ecCodes monthly
  concept space represented by `672`, `696`, `720`, `744` hours.
- The step-zero statistical-processing allowance should become a precomputed
  `ProductTimeSpecInput` member.
- `TypeOfStatisticalProcessing::Missing` must be treated as always valid in the
  step-zero allowance rule because it represents the instant case.
- Detailed paranoia/check-level options are intentionally deferred.

## Active Rewrite Order

1. update the living markdown to reflect the completed callback and checker work;
2. add the raw-to-normalized artifact layer;
3. add post-normalization cross checks;
4. audit diagnostic JSON and exception boundaries;
5. harden `TemporalArithmetic`;
6. rewrite in-code documentation.

## Callback Review Tracker

Use the following status values:

- `pending`
- `reviewed`
- `rewrite_started`
- `rewritten`
- `checked`
- `normalized_path_reviewed`
- `cross_checked`

### Anchor callbacks

- `match_ForecastAnalysis_Anchor` : checked
- `build_ForecastAnalysis_Anchor` : checked
- `match_Hindcast_Anchor` : checked
- `build_Hindcast_Anchor` : checked
- `match_SeasonalClimate_Anchor` : checked
- `build_SeasonalClimate_Anchor` : checked

### Domain callbacks

- `match_Analysis_Domain` : checked
- `build_Analysis_Domain` : checked
- `match_Forecast_Domain` : checked
- `build_Forecast_Domain` : checked
- `match_FromStartForecast_Domain` : checked
- `build_FromStartForecast_Domain` : checked
- `match_SeasonalForecast_Domain` : checked
- `build_SeasonalForecast_Domain` : checked
- `match_SynopticAnalysis_Domain` : checked
- `build_SynopticAnalysis_Domain` : checked

### Shape callbacks

- `match_Instant_Shape` : checked
- `build_Instant_ShapeOuterTimeRange` : checked
- `build_Instant_ShapeWindows` : checked
- `match_IFSStandardSingleLoop_Shape` : checked
- `build_IFSStandardSingleLoop_ShapeOuterTimeRange` : checked
- `build_IFSStandardSingleLoop_ShapeWindows` : checked
- `match_IFSFakeDoubleLoopSingleLoop_Shape` : checked
- `build_IFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange` : checked
- `build_IFSFakeDoubleLoopSingleLoop_ShapeWindows` : checked
- `match_IFSFromStartSingleLoopAtZero_Shape` : checked
- `build_IFSFromStartSingleLoopAtZero_ShapeOuterTimeRange` : checked
- `build_IFSFromStartSingleLoopAtZero_ShapeWindows` : checked
- `match_IFSFromStartSingleLoopPositive_Shape` : checked
- `build_IFSFromStartSingleLoopPositive_ShapeOuterTimeRange` : checked
- `build_IFSFromStartSingleLoopPositive_ShapeWindows` : checked
- `match_IFSSynopticSingleLoop_Shape` : checked
- `build_IFSSynopticSingleLoop_ShapeOuterTimeRange` : checked
- `build_IFSSynopticSingleLoop_ShapeWindows` : checked
- `match_AIFSStandardSingleLoop_Shape` : checked
- `build_AIFSStandardSingleLoop_ShapeOuterTimeRange` : checked
- `build_AIFSStandardSingleLoop_ShapeWindows` : checked
- `match_AIFSFakeDoubleLoopSingleLoop_Shape` : checked
- `build_AIFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange` : checked
- `build_AIFSFakeDoubleLoopSingleLoop_ShapeWindows` : checked
- `match_AIFSFromStartSingleLoopAtZero_Shape` : checked
- `build_AIFSFromStartSingleLoopAtZero_ShapeOuterTimeRange` : checked
- `build_AIFSFromStartSingleLoopAtZero_ShapeWindows` : checked
- `match_AIFSFromStartSingleLoopPositive_Shape` : checked
- `build_AIFSFromStartSingleLoopPositive_ShapeOuterTimeRange` : checked
- `build_AIFSFromStartSingleLoopPositive_ShapeWindows` : checked
- `match_SeasonalSingleLoop_Shape` : checked
- `build_SeasonalSingleLoop_ShapeOuterTimeRange` : checked
- `build_SeasonalSingleLoop_ShapeWindows` : checked
- `match_SeasonalMultiloop_Shape` : checked
- `build_SeasonalMultiloop_ShapeOuterTimeRange` : checked
- `build_SeasonalMultiloop_ShapeWindows` : checked
- `match_IFSStandardMultiLoop_Shape` : checked
- `build_IFSStandardMultiLoop_ShapeOuterTimeRange` : checked
- `build_IFSStandardMultiLoop_ShapeWindows` : checked
- `match_IFSFakeSingleLoopDoubleLoop_Shape` : checked
- `build_IFSFakeSingleLoopDoubleLoop_ShapeOuterTimeRange` : checked
- `build_IFSFakeSingleLoopDoubleLoop_ShapeWindows` : checked

## Future Raw Check Callback Tracker

### Anchor raw checks

- `check_ForecastAnalysis_Anchor` : added
- `check_Hindcast_Anchor` : added
- `check_SeasonalClimate_Anchor` : added

### Domain raw checks

- `check_Analysis_Domain` : added
- `check_Forecast_Domain` : added
- `check_FromStartForecast_Domain` : added
- `check_SeasonalForecast_Domain` : added
- `check_SynopticAnalysis_Domain` : added

### Shape raw checks

- `check_Instant_Shape` : added
- `check_IFSStandardSingleLoop_Shape` : added
- `check_IFSFakeDoubleLoopSingleLoop_Shape` : added
- `check_IFSFromStartSingleLoopAtZero_Shape` : added
- `check_IFSFromStartSingleLoopPositive_Shape` : added
- `check_IFSSynopticSingleLoop_Shape` : added
- `check_AIFSStandardSingleLoop_Shape` : added
- `check_AIFSFakeDoubleLoopSingleLoop_Shape` : added
- `check_AIFSFromStartSingleLoopAtZero_Shape` : added
- `check_AIFSFromStartSingleLoopPositive_Shape` : added
- `check_SeasonalSingleLoop_Shape` : added
- `check_SeasonalMultiloop_Shape` : added
- `check_IFSStandardMultiLoop_Shape` : added
- `check_IFSFakeSingleLoopDoubleLoop_Shape` : added

## Normalization Tracker

Target invariants:

- all final time ranges in hours;
- all final time increments in seconds or missing;
- monthly values normalized from real placed intervals;
- normalized hour values validated against the agreed whitelists.

Current status:

- helper added in `detail/ShapeNormalization.h`, top-level wiring not implemented yet.

## Early Input-Contract Refinement Tracker

Target change:

- update the current step-zero allowance helper so
  `TypeOfStatisticalProcessing::Missing` is always valid;
- add the derived allowance as a dedicated `ProductTimeSpecInput` member;
- consume the member from the zero-step shape callbacks instead of recomputing
  the rule locally.

Current status:

- implemented.

## Cross-Check Tracker

Planned cross checks include:

- domain offsets versus real reference-based datetime differences;
- synoptic cross consistency;
- seasonal cross consistency;
- domain/span/window consistency;
- normalized whitelist enforcement after artifact interaction is known.

Current status:

- not implemented.

## Error-Handling Tracker

Target state:

- all diagnostic JSON best-effort and `noexcept`;
- all nontrivial code wrapped in full-body `try/catch` with nested rethrow.

Current status:

- audit not started.

## `TemporalArithmetic` Tracker

Target state:

- strong overflow and underflow protection;
- month-safe arithmetic;
- exact whole-hour support for normalization and domain offsets;
- readable and layered failure modes.

Current status:

- review not started.

## Deferred Decisions

Still intentionally deferred:

- detailed paranoia/check option structure;
- final public-facing markdown structure after the rewrite stabilizes.

## Next Session Entry Point

The next session should begin by:

1. updating the living markdown to reflect the completed callback and checker work;
2. adding the raw-to-normalized artifact layer;
3. wiring normalization into the top-level ProductTimeSpec pipeline;
4. adding final cross checks on normalized artifacts;
5. only then beginning the exception-boundary and arithmetic hardening phases.
