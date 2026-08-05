<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Shape Cases

## Overview

Shape classification chooses the canonical statistical-window topology used in
the final ProductTimeSpec representation.

Classification is exhaustive and non-prioritized. Exactly one matcher must
return `true`.

## Callback Matrix

| Case | Regime | Domain kind | Synoptic | `timespan.kind` | `stattype` | `timeIncrement` expectation | Fake-double-loop flag | Fake-second-loop flag |
|---|---|---|---|---|---|---|---|---|
| `Instant` | any | any | any | `None`, or `Missing` when allowed | empty | redundant values validated later | n/a | n/a |
| `IFSStandardSingleLoop` | `IFS` | `ForecastDomain` | `false` | `Duration` | empty | explicit, missing, or defaulted | `false` | `false` |
| `IFSFakeDoubleLoopSingleLoop` | `IFS` | `ForecastDomain` | `false` | `None`, or `Missing` when allowed | exactly one block | explicit, missing, or defaulted | `true` | `false` |
| `IFSFromStartSingleLoopAtZero` | `IFS` | `ForecastDomain` | `false` | `FromStart` | empty | explicit, missing, or defaulted | n/a | n/a |
| `IFSFromStartSingleLoopPositive` | `IFS` | `ForecastDomain` | `false` | `FromStart` | empty | explicit, missing, or defaulted | n/a | n/a |
| `IFSSynopticSingleLoop` | `IFS` | `SynopticAnalysisDomain` | `true` | synoptic-supported source | empty | intrinsic or redundant 24h value | n/a | n/a |
| `AIFSStandardSingleLoop` | `AIFS` | `ForecastDomain` | `false` | `Duration` | empty | must be missing | `false` | `false` |
| `AIFSFakeDoubleLoopSingleLoop` | `AIFS` | `ForecastDomain` | `false` | `None`, or `Missing` when allowed | exactly one block | must be missing | `true` | n/a |
| `AIFSFromStartSingleLoopAtZero` | `AIFS` | `ForecastDomain` | `false` | `FromStart` | empty | must be missing | n/a | n/a |
| `AIFSFromStartSingleLoopPositive` | `AIFS` | `ForecastDomain` | `false` | `FromStart` | empty | must be missing | n/a | n/a |
| `SeasonalSingleLoop` | any | `SeasonalForecastDomain` | `false` | `None`, or `Missing` when allowed | empty | explicit, missing, or defaulted | n/a | n/a |
| `SeasonalMultiloop` | any | `SeasonalForecastDomain` | `false` | `Duration` | one or more blocks | explicit, missing, or defaulted | n/a | n/a |
| `IFSStandardMultiLoop` | `IFS` | `ForecastDomain` | `false` | `Duration` | one or more blocks | explicit, missing, or defaulted | n/a | n/a |
| `IFSFakeSingleLoopDoubleLoop` | `IFS` | `ForecastDomain` | `false` | `Duration` | empty | explicit, missing, or defaulted | `false` | `true` |

## Notes

- Shape classification depends on the already resolved domain kind.
- AIFS cases currently exist only for forecast-domain products.
- All non-seasonal forecast-like shapes explicitly reject seasonal input through `isSeasonal(input)`.
- `SeasonalForecastDomain` is currently accepted by `SeasonalSingleLoop` and `SeasonalMultiloop`.
- Seasonal multi-loop currently classifies but its builder intentionally throws `not implemented`.
- Seasonal single-loop windows always use a one-calendar-month canonical range; `fcmonth` places the statistics at the relevant forecast month.
- Missing `timespan` is accepted for fake-double-loop and seasonal single-loop statistics only when `allowMissingTimespanForStatisticalProduct` is enabled.
