<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# ProductTimeSpec V5 case-based implementation — revision 4

## Architecture

The same case-based pattern is used for anchors, domains, and shapes:

1. one file per semantic case;
2. one matcher and one builder in that file;
3. one immutable registry per classification axis;
4. every matcher is evaluated and its Boolean result is retained;
5. classification succeeds only when exactly one matcher returns `true`.

Matcher order is never used as precedence. Zero matches and overlapping matches
are both hard classification failures.

Construction follows the dependency order:

```text
classify anchor, domain, shape
build anchor
build domain(input, anchor)
build shape(input, domain)
```

## Leaf-builder rule

Anchor, domain, and shape case files own their high-level logic. In particular,
shape builders are leaves: range selection, shape-specific validation, increment
resolution, window construction, and window ordering remain visible in the shape
file.

Delegation is limited to genuinely cross-cutting semantics:

- temporal arithmetic;
- normalized primitive extraction;
- forecast-lead predicates shared by multiple seasonal matchers and domain
  builders;
- `typeOfTimeIncrement` resolution;
- explicit increment validation;
- default time-increment deduction;
- shared product-rule lookups.

Generic `makeWindow`, `buildSingleLoop`, or `buildMultiLoopWindows` helpers are
intentionally avoided.

## Seasonal forecast split

Seasonal forecast support is currently isolated at the domain level.

The normalized seasonal discriminator is centralized in:

- `detail/ForecastLeadUtils.h::isSeasonal(...)`

Its semantics are exact and intentionally local:

```text
step is missing
fcmonth is present
```

Forecast domains are therefore split into two explicit cases:

- `ForecastDomain`, for non-seasonal forecast products whose lead comes from
  `step`;
- `SeasonalForecastDomain`, for seasonal forecast products whose lead comes from
  `fcmonth` and is represented as calendar months.

Seasonal forecast outer-range resolution also has a dedicated callback path so
future seasonal divergence can remain local to the seasonal domain case even
when the current implementation is still close to the non-seasonal forecast
path.

## Naming convention

- Files, enumeration values, and semantic cases use `PascalCase`.
- Matchers use `match_<Case>_<Category>`.
- Builders use `build_<Case>_<Category>`.
- Registry classifiers use `classify_<Category>_or_throw`.
- Registry dispatchers use `build_<Category>_or_throw`.
- Matcher conditions use semantic names such as `isAifs`,
  `hasForecastDomain`, and `hasNoStattypeBlocks`.

## Exception contract

Every function:

1. imports `Mars2GribModelException` locally with a `using` declaration;
2. wraps its complete body in `try` / `catch (...)`;
3. rethrows directly in the catch block with `std::throw_with_nested(...)`.

Functions receiving `ProductTimeSpecInput` use:

```text
Mars2GribModelException(reason, input.to_json(), Here())
```

Lower-level functions without an input snapshot use:

```text
Mars2GribModelException(reason, Here())
```

There is no exception adapter helper and no intentional standard exception type
is thrown.

## C++ version

The headers require C++17 and contain no designated initializers or other C++20
language features.

## Deliberately incomplete integration points

- `detail/ProductRules.h::fakeSingleLoopDoubleLoopRules` remains empty until the
  authoritative `(class, stream, type, paramId)` table is supplied.
- `detail/ShapeUtils.h::deduceDefaultTimeIncrement(...)` is the isolated extension
  point for the final complex defaulting algorithm.
- `detail/TemporalArithmetic.h` contains explicit calendar-month integration
  points that must be connected to the repository calendar utility.

## Expected repository types

The implementation expects the existing ProductTimeSpec input, anchor, domain,
window, duration, option, and table types.
