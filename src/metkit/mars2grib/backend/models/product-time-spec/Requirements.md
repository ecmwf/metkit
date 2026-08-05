<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Seasonal Forecast Requirements

## Scope

This document captures the requirements for seasonal forecast support in the
`product-time-spec` backend model.

For the current iteration, the priority is **SeasonalForecast** only.
`Climate` is explicitly out of scope except where noted for future separation.

## Core Rule

A **SeasonalForecast** behaves like a normal forecast, except that the forecast
lead is provided by `fcmonth` instead of `step`.

Semantically:

- normal forecast lead source: `step`
- seasonal forecast lead source: `fcmonth`

## Seasonal Discriminator

The normalized input is considered **seasonal** when:

- `step` is missing
- `fcmonth` is present

A shared helper shall expose this rule as:

- `isSeasonal(input)`

Its implementation shall remain exactly:

```cpp
!input.step.has_value() && input.marsFcmonth.has_value()
```

## `fcmonth` Semantics

`fcmonth` is not a calendar-month enumerator.

It represents:

- a positive integer forecast lead count
- with unit always equal to `Month`

Examples:

- `fcmonth=1` means one month after reference time
- `fcmonth=2` means two months after reference time
- `fcmonth=18` means eighteen months after reference time

`fcmonth` must therefore:

- accept positive integers
- not be artificially bounded to `[1,12]`

## Domain Semantics

Seasonal forecast support is a **domain-level modification** and is orthogonal
to anchor logic.

The forecast domain rule becomes:

- normal forecast: `domainEndDateTime = referenceDateTime + step`
- seasonal forecast: `domainEndDateTime = referenceDateTime + fcmonth months`

The domain start rule remains unchanged:

- `domainStartDateTime = domainEndDateTime - outerRange`

## Shape Semantics

A new shape case shall be introduced:

- `SeasonalSingleLoop`

Seasonal support also reserves:

- `SeasonalMultiloop`

The currently implemented seasonal window semantics remain monthly.

Its matcher shall use `isSeasonal(input)` and the normal single-loop structural
checks.

At minimum, the single-loop matcher is expected to verify:

- `isSeasonal(input)`
- forecast semantics
- non-synoptic semantics
- `timespan` is `none`, or missing when allowed
- no `stattype` blocks

The single-loop canonical window range is always one calendar month. `fcmonth`
places that monthly statistic at the relevant forecast month.

The seasonal discriminator itself shall remain centralized in the helper layer,
not duplicated in shape matchers.

## Helper Layer

A shared helper header shall be introduced under `detail/` to avoid duplicating
raw `step`/`fcmonth` checks across domains and shapes.

Expected helper responsibilities include:

- `hasStep(input)`
- `hasFcmonth(input)`
- `stepIsMissing(input)`
- `fcmonthIsPresent(input)`
- `isSeasonal(input)`

Additional helpers may be added if needed for lead resolution.

## Time Representation

Seasonal forecast lead must preserve month semantics.

Lead values derived from `fcmonth` shall be materialized as:

- `TimeDuration{N, TimeUnit::Month}`

Calendar months must not be approximated with a fixed number of seconds.

## Non-Goals For This Iteration

The following are not part of the current seasonal work:

- clarifying or redesigning `Climate`
- anchor redesign for climate-like products
- introducing day-based concept-level forecast time support
- broadening seasonal support beyond the current single-loop path

## Constraints

- The implementation should prefer small, explicit changes.
- Seasonal detection must be centralized behind `isSeasonal(input)`.
- Domain logic must stop assuming that forecast lead always comes from `step`.
- Existing non-seasonal behavior must remain unchanged.
