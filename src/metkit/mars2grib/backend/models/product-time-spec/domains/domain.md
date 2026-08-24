<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Domain Logic

## Purpose

The domain layer resolves the absolute temporal support interval used by the
rest of `ProductTimeSpec`.

It produces one resolved `ProductTimeSpecDomain` artifact containing:

- `domainStartDateTime`
- `domainEndDateTime`
- `isSynoptic`
- `startOffsetHoursFromReference`
- `endOffsetHoursFromReference`

These values are derived from the normalized input, the resolved anchor, and
the stage-1 outer time range.

## Folder Structure

The current domain folder contains:

- `DomainDataTypes.h`
- `DomainRegistry.h`
- `DomainUtils.h`
- `impl/AnalysisDomain.h`
- `impl/ForecastDomain.h`
- `impl/FromStartForecastDomain.h`
- `impl/SeasonalForecastDomain.h`
- `impl/SynopticAnalysisDomain.h`

## Resolved Domain Artifact

The resolved artifact type is:

- `domain::ProductTimeSpecDomain`

It stores:

- the absolute support start datetime;
- the absolute support end datetime;
- whether the domain uses synoptic placement semantics;
- the signed whole-hour offset from the anchor reference datetime to the real
  support start;
- the signed whole-hour offset from the anchor reference datetime to the
  support end.

## Classification Flow

The domain classification entry point is:

- `domain::classify_Domain_or_throw(...)`

It is implemented in:

- `DomainRegistry.h`

Behavior:

1. evaluate every registered matcher;
2. count how many matchers returned `true`;
3. succeed only when exactly one matcher matches;
4. otherwise throw a classification failure.

Matcher order is not precedence.

## Builder Flow

The domain builder dispatch entry point is:

- `domain::build_Domain_or_throw(...)`

It is implemented in:

- `DomainRegistry.h`

Behavior:

1. validate the classification enum value;
2. dispatch to the registered per-case builder;
3. return the resolved `ProductTimeSpecDomain`.

## Checker Flow

The domain checker dispatch entry point is:

- `domain::check_Domain_or_throw(...)`

It is implemented in:

- `DomainRegistry.h`

Behavior:

1. validate the classification enum value;
2. dispatch to the registered per-case checker;
3. return `true` on success;
4. otherwise throw a check failure.

The checker infrastructure is now present for all currently implemented domain
cases. Top-level orchestration may still need to call the checker stage
explicitly in a later integration phase.

## Shared Utility Layer

`DomainUtils.h` contains low-level helpers used by multiple domain and shape
cases.

Current responsibilities include:

- direct normalized non-seasonal lead extraction;
- direct normalized seasonal lead extraction;
- conversion of duration-valued `timespan` into seconds;
- shared outer-range resolution;
- conversion of two absolute datetimes into a signed whole-hour offset.

The key low-level helper for the current raw domain model is:

- `offsetHoursFromReference(...)`

It converts the difference between two datetimes into a signed whole-hour
offset, which is what the raw domain artifact stores for encoder-facing use.

## Current Cases

### AnalysisDomain

File:

- `impl/AnalysisDomain.h`

Matcher:

- `match_Analysis_Domain(...)`

The case matches when:

- the product is not synoptic;
- the regime is not AIFS;
- the simulation type is analysis.

Builder:

- `build_Analysis_Domain(...)`

Construction rule:

- `domainStartDateTime = anchor.referenceDateTime`
- `domainEndDateTime = anchor.referenceDateTime + outerRange`
- `isSynoptic = false`
- offsets are measured from `anchor.referenceDateTime`

Checker:

- `check_Analysis_Domain(...)`

The checker validates:

- non-synoptic semantics;
- support start equals anchor reference;
- support start does not follow support end;
- offsets match the resolved datetimes.

### ForecastDomain

File:

- `impl/ForecastDomain.h`

Matcher:

- `match_Forecast_Domain(...)`

The case matches when:

- the product is not synoptic;
- the product is not seasonal;
- the simulation type is forecast;
- `timespan.kind` is not `FromStart`.

Builder:

- `build_Forecast_Domain(...)`

Construction rule:

- `domainEndDateTime = anchor.referenceDateTime + step`
- `domainStartDateTime = domainEndDateTime - outerRange`
- `isSynoptic = false`
- offsets are measured from `anchor.referenceDateTime`

Checker:

- `check_Forecast_Domain(...)`

The checker validates:

- non-synoptic semantics;
- support start does not follow support end;
- support end does not precede anchor reference;
- offsets match the resolved datetimes.

### FromStartForecastDomain

File:

- `impl/FromStartForecastDomain.h`

Matcher:

- `match_FromStartForecast_Domain(...)`

The case matches when:

- the product is not synoptic;
- the product is not seasonal;
- the simulation type is forecast;
- `timespan.kind` is `FromStart`.

Builder:

- `build_FromStartForecast_Domain(...)`

Construction rule:

- the outer time range must remain deferred at the stage-1 interface;
- `domainEndDateTime = anchor.referenceDateTime + step`
- `domainStartDateTime = domainEndDateTime - step`
- under the current semantics this resolves to:
  - `domainStartDateTime = anchor.referenceDateTime`
- `isSynoptic = false`
- offsets are measured from `anchor.referenceDateTime`

Checker:

- `check_FromStartForecast_Domain(...)`

The checker validates:

- non-synoptic semantics;
- support start does not follow support end;
- support start equals anchor reference;
- offsets match the resolved datetimes.

### SeasonalForecastDomain

File:

- `impl/SeasonalForecastDomain.h`

Matcher:

- `match_SeasonalForecast_Domain(...)`

The case matches when:

- the product is not synoptic;
- the normalized input is seasonal;
- the simulation type is forecast.

Builder:

- `build_SeasonalForecast_Domain(...)`

Construction rule:

- validate seasonal semantics explicitly in the callback;
- validate `fcmonth` presence explicitly in the callback;
- validate `fcmonth > 0` explicitly in the callback;
- build a month-valued lead as `TimeDuration{fcmonth, TimeUnit::Month}`;
- `domainEndDateTime = anchor.referenceDateTime + forecastLead`
- `domainStartDateTime = domainEndDateTime - outerRange`
- `isSynoptic = false`
- offsets are measured from `anchor.referenceDateTime`

Checker:

- `check_SeasonalForecast_Domain(...)`

The checker validates:

- non-synoptic semantics;
- support start does not follow support end;
- support end does not precede anchor reference;
- offsets match the resolved datetimes.

### SynopticAnalysisDomain

File:

- `impl/SynopticAnalysisDomain.h`

Matcher:

- `match_SynopticAnalysis_Domain(...)`

The case matches when:

- the product is synoptic;
- the regime is IFS;
- the simulation type is analysis.

Builder:

- `build_SynopticAnalysis_Domain(...)`

Construction rule:

- `domainStartDateTime` preserves the exact MARS date/time and therefore keeps
  the synoptic hour in its time component;
- the real support start used for hour offsets is the same date forced to
  `00:00:00`;
- `domainEndDateTime` is the first instant of the following calendar month
  computed from that real support start;
- `isSynoptic = true`
- start offset is computed from the real midnight-based support start, not from
  the stored synoptic timestamp;
- end offset is computed from the support end.

Checker:

- `check_SynopticAnalysis_Domain(...)`

The checker validates:

- synoptic semantics;
- input MARS date presence;
- start date agreement with the input MARS date;
- support start does not follow support end;
- support end equals the next calendar-month boundary computed from the real
  midnight-based support start;
- offsets match the resolved real support start and support end.

## Diagnostic JSON

`DomainDataTypes.h` provides:

- `productTimeSpecDomainJson(...)`

This function is:

- best-effort;
- `noexcept`;
- intended only for diagnostic context construction.

If serialization fails, it returns a stable fallback JSON error object instead
of throwing.

## Current Limitations

The domain checker infrastructure is present and the registry now exposes the
checker dispatch entry point.

The remaining major work after the callback rewrite is now the raw-to-
normalized artifact layer and the later post-normalization cross checks.

During normalization, the domain artifact remains unchanged and supplies the
real placement used to normalize month-based shape ranges to hours.
