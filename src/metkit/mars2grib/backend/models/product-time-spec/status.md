<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Seasonal Forecast Status

## Current Understanding

The current priority is **SeasonalForecast** only.

Working definition:

- seasonal forecast is a forecast-like product
- it uses `fcmonth` instead of `step`
- it is considered seasonal when `step` is missing and `fcmonth` is present

Reference helper now introduced in `detail/ForecastLeadUtils.h`:

```cpp
isSeasonal(input) == !input.step.has_value() && input.marsFcmonth.has_value()
```

## What Already Exists

The codebase already contains:

- normalized optional `marsFcmonth` in `ProductTimeSpecInput`
- month-aware calendar arithmetic in `utils/TemporalArithmetic.h`
- forecast domain logic that can be generalized
- shape registry structure that can host a new seasonal shape

Useful existing support:

- `ProductTimeSpecInput.h` already reads `fcmonth`
- `TemporalArithmetic.h` already supports `Month` in `addDuration()` and `subtractDuration()`

## Current Gaps

### 1. `fcmonth` deduction semantics have been updated

`backend/deductions/fcmonth.h` now treats `fcmonth` as a strictly positive
integer forecast lead count.

The old bounded calendar-month enum interpretation has been removed.

Completed change:

- reinterpret `fcmonth` as a positive integer lead count
- remove the `[1,12]` restriction

### 2. Forecast domain logic is being split by domain type

`backend/models/product-time-spec/domains/DomainUtils.h` and
`domains/impl/ForecastDomain.h` no longer need to own both non-seasonal and
seasonal forecast semantics in one callback.

Current direction:

- keep `ForecastDomain` for non-seasonal forecast products
- add `SeasonalForecastDomain` for seasonal forecast products
- keep seasonal outer-range logic isolated so it can diverge later if needed

### 3. Shape matchers still contain raw step logic

The current shape layer contains repeated direct checks on `input.step`.

Notable files:

- `shapes/impl/IFSFromStartSingleLoopAtZero.h`
- `shapes/impl/IFSFromStartSingleLoopPositive.h`
- `shapes/impl/AIFSFromStartSingleLoopAtZero.h`
- `shapes/impl/AIFSFromStartSingleLoopPositive.h`

Completed groundwork:

- introduced a shared helper layer in `detail/ForecastLeadUtils.h`
- added `hasStep`, `hasFcmonth`, `stepIsMissing`, `fcmonthIsPresent`, and `isSeasonal`

Remaining change:

- stop duplicating raw step/fcmonth predicates

### 4. Seasonal shape does not exist yet

There is currently no `SeasonalSingleLoop` case in:

- `ShapeDataTypes.h`
- `ShapeRegistry.h`

Required change:

- add `SeasonalSingleLoop`
- implement matcher/builder file

## Files Expected To Change

### High priority

- `backend/models/product-time-spec/ProductTimeSpecInput.h`
- `backend/models/product-time-spec/domains/DomainUtils.h`
- `backend/models/product-time-spec/domains/impl/ForecastDomain.h`
- `backend/models/product-time-spec/domains/impl/SeasonalForecastDomain.h`
- `backend/models/product-time-spec/domains/DomainDataTypes.h`
- `backend/models/product-time-spec/domains/DomainRegistry.h`
- `backend/models/product-time-spec/shapes/ShapeDataTypes.h`
- `backend/models/product-time-spec/shapes/ShapeRegistry.h`
- `backend/models/product-time-spec/shapes/impl/SeasonalSingleLoop.h`

### Medium priority cleanup

- `shapes/impl/IFSFromStartSingleLoopAtZero.h`
- `shapes/impl/IFSFromStartSingleLoopPositive.h`
- `shapes/impl/AIFSFromStartSingleLoopAtZero.h`
- `shapes/impl/AIFSFromStartSingleLoopPositive.h`

### Documentation

- `README.md`
- `domains/domain.md`
- `shapes/shapes.md`

## Things That Are Not Blockers

Seasonal forecast domain support is not blocked by anchor logic.

For the current iteration:

- seasonal is treated as forecast-domain behavior
- climate remains separate and deferred

## Open Technical Risk

The new `SeasonalSingleLoop` matcher must be specific enough to avoid shape
classification overlap.

`isSeasonal(input)` alone is not sufficient as a full matcher.

The matcher will also need the normal structural single-loop constraints, such
as:

- forecast semantics
- non-synoptic semantics
- duration-valued `timespan`
- no `stattype` blocks

## Recommended Next Steps

1. Review shape/domain-kind interactions now that `SeasonalForecastDomain` exists.
2. Add `SeasonalSingleLoop`.
3. Replace duplicated raw step checks with helper calls.
4. Update remaining documentation.
