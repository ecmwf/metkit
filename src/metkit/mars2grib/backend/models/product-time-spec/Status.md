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
- one additional early normalized-input refinement has been identified before
  the callback-by-callback rewrite;
- code not modified yet under this rewrite plan;
- callback-by-callback review not started yet.

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

1. extend raw domain artifact;
2. extend normalized input with precomputed step-zero allowance;
3. review and refactor anchor callbacks;
4. review and refactor domain callbacks;
5. review and refactor shape callbacks;
6. add per-case raw check callbacks;
7. add raw-to-normalized artifact layer;
8. add post-normalization cross checks;
9. audit diagnostic JSON and exception boundaries;
10. harden `TemporalArithmetic`;
11. rewrite in-code documentation.

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

- `match_ForecastAnalysis_Anchor` : pending
- `build_ForecastAnalysis_Anchor` : pending
- `match_Hindcast_Anchor` : pending
- `build_Hindcast_Anchor` : pending
- `match_SeasonalClimate_Anchor` : pending
- `build_SeasonalClimate_Anchor` : pending

### Domain callbacks

- `match_Analysis_Domain` : pending
- `build_Analysis_Domain` : pending
- `match_Forecast_Domain` : pending
- `build_Forecast_Domain` : pending
- `match_FromStartForecast_Domain` : pending
- `build_FromStartForecast_Domain` : pending
- `match_SeasonalForecast_Domain` : pending
- `build_SeasonalForecast_Domain` : pending
- `match_SynopticAnalysis_Domain` : pending
- `build_SynopticAnalysis_Domain` : pending

### Shape callbacks

- `match_Instant_Shape` : pending
- `build_Instant_ShapeOuterTimeRange` : pending
- `build_Instant_ShapeWindows` : pending
- `match_IFSStandardSingleLoop_Shape` : pending
- `build_IFSStandardSingleLoop_ShapeOuterTimeRange` : pending
- `build_IFSStandardSingleLoop_ShapeWindows` : pending
- `match_IFSFakeDoubleLoopSingleLoop_Shape` : pending
- `build_IFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange` : pending
- `build_IFSFakeDoubleLoopSingleLoop_ShapeWindows` : pending
- `match_IFSFromStartSingleLoopAtZero_Shape` : pending
- `build_IFSFromStartSingleLoopAtZero_ShapeOuterTimeRange` : pending
- `build_IFSFromStartSingleLoopAtZero_ShapeWindows` : pending
- `match_IFSFromStartSingleLoopPositive_Shape` : pending
- `build_IFSFromStartSingleLoopPositive_ShapeOuterTimeRange` : pending
- `build_IFSFromStartSingleLoopPositive_ShapeWindows` : pending
- `match_IFSSynopticSingleLoop_Shape` : pending
- `build_IFSSynopticSingleLoop_ShapeOuterTimeRange` : pending
- `build_IFSSynopticSingleLoop_ShapeWindows` : pending
- `match_AIFSStandardSingleLoop_Shape` : pending
- `build_AIFSStandardSingleLoop_ShapeOuterTimeRange` : pending
- `build_AIFSStandardSingleLoop_ShapeWindows` : pending
- `match_AIFSFakeDoubleLoopSingleLoop_Shape` : pending
- `build_AIFSFakeDoubleLoopSingleLoop_ShapeOuterTimeRange` : pending
- `build_AIFSFakeDoubleLoopSingleLoop_ShapeWindows` : pending
- `match_AIFSFromStartSingleLoopAtZero_Shape` : pending
- `build_AIFSFromStartSingleLoopAtZero_ShapeOuterTimeRange` : pending
- `build_AIFSFromStartSingleLoopAtZero_ShapeWindows` : pending
- `match_AIFSFromStartSingleLoopPositive_Shape` : pending
- `build_AIFSFromStartSingleLoopPositive_ShapeOuterTimeRange` : pending
- `build_AIFSFromStartSingleLoopPositive_ShapeWindows` : pending
- `match_SeasonalSingleLoop_Shape` : pending
- `build_SeasonalSingleLoop_ShapeOuterTimeRange` : pending
- `build_SeasonalSingleLoop_ShapeWindows` : pending
- `match_SeasonalMultiloop_Shape` : pending
- `build_SeasonalMultiloop_ShapeOuterTimeRange` : pending
- `build_SeasonalMultiloop_ShapeWindows` : pending
- `match_IFSStandardMultiLoop_Shape` : pending
- `build_IFSStandardMultiLoop_ShapeOuterTimeRange` : pending
- `build_IFSStandardMultiLoop_ShapeWindows` : pending
- `match_IFSFakeSingleLoopDoubleLoop_Shape` : pending
- `build_IFSFakeSingleLoopDoubleLoop_ShapeOuterTimeRange` : pending
- `build_IFSFakeSingleLoopDoubleLoop_ShapeWindows` : pending

## Future Raw Check Callback Tracker

### Anchor raw checks

- `check_ForecastAnalysis_Anchor` : not added
- `check_Hindcast_Anchor` : not added
- `check_SeasonalClimate_Anchor` : not added

### Domain raw checks

- `check_Analysis_Domain` : not added
- `check_Forecast_Domain` : not added
- `check_FromStartForecast_Domain` : not added
- `check_SeasonalForecast_Domain` : not added
- `check_SynopticAnalysis_Domain` : not added

### Shape raw checks

- `check_Instant_Shape` : not added
- `check_IFSStandardSingleLoop_Shape` : not added
- `check_IFSFakeDoubleLoopSingleLoop_Shape` : not added
- `check_IFSFromStartSingleLoopAtZero_Shape` : not added
- `check_IFSFromStartSingleLoopPositive_Shape` : not added
- `check_IFSSynopticSingleLoop_Shape` : not added
- `check_AIFSStandardSingleLoop_Shape` : not added
- `check_AIFSFakeDoubleLoopSingleLoop_Shape` : not added
- `check_AIFSFromStartSingleLoopAtZero_Shape` : not added
- `check_AIFSFromStartSingleLoopPositive_Shape` : not added
- `check_SeasonalSingleLoop_Shape` : not added
- `check_SeasonalMultiloop_Shape` : not added
- `check_IFSStandardMultiLoop_Shape` : not added
- `check_IFSFakeSingleLoopDoubleLoop_Shape` : not added

## Normalization Tracker

Target invariants:

- all final time ranges in hours;
- all final time increments in seconds or missing;
- monthly values normalized from real placed intervals;
- normalized hour values validated against the agreed whitelists.

Current status:

- not implemented.

## Early Input-Contract Refinement Tracker

Target change:

- update the current step-zero allowance helper so
  `TypeOfStatisticalProcessing::Missing` is always valid;
- add the derived allowance as a dedicated `ProductTimeSpecInput` member;
- consume the member from the zero-step shape callbacks instead of recomputing
  the rule locally.

Current status:

- identified;
- not implemented.

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

1. extending `ProductTimeSpecInput` with the precomputed step-zero allowance;
2. extending `ProductTimeSpecDomain`;
3. reviewing domain callbacks one by one;
4. deciding the raw-to-normalized domain and shape artifact boundary;
5. only then beginning code modifications.
