<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Anchor Logic

## Purpose

The anchor layer resolves the temporal anchor state used by the rest of
`ProductTimeSpec`.

It produces one resolved `ProductTimeSpecAnchor` artifact containing:

- `labelDateTime`
- `initialConditionsDateTime`
- `referenceDateTime`
- `anchorType`

These values are derived directly from the normalized input and are later used
by domain and shape construction.

## Folder Structure

The current anchor folder contains:

- `AnchorDataTypes.h`
- `AnchorRegistry.h`
- `impl/ForecastAnalysis.h`
- `impl/Hindcast.h`
- `impl/SeasonalClimate.h`

There is currently no remaining shared `AnchorUtils.h`. The only shared
low-level anchor invariant helper is now `checkedAnchor(...)` in
`AnchorDataTypes.h`.

## Resolved Anchor Artifact

The resolved artifact type is:

- `anchor::ProductTimeSpecAnchor`

It stores three absolute datetimes and the producing anchor classification.

The canonical low-level invariant is:

```text
labelDateTime <= initialConditionsDateTime <= referenceDateTime
```

This invariant is validated by:

- `anchor::checkedAnchor(...)`

`checkedAnchor(...)` is intentionally input-free. It validates the invariant and
constructs the final anchor artifact. It throws a low-level generic exception.
Higher-level callback code catches and rethrows with richer model-input context.

## Classification Flow

The anchor classification entry point is:

- `anchor::classify_Anchor_or_throw(...)`

It is implemented in:

- `AnchorRegistry.h`

Behavior:

1. evaluate every registered matcher;
2. count how many matchers returned `true`;
3. succeed only when exactly one matcher matches;
4. otherwise throw a classification failure.

Matcher order is not precedence.

The current registered cases are:

- `ForecastAnalysis`
- `Hindcast`
- `SeasonalClimate`

## Builder Flow

The anchor builder dispatch entry point is:

- `anchor::build_Anchor_or_throw(...)`

It is implemented in:

- `AnchorRegistry.h`

Behavior:

1. validate the classification enum value;
2. dispatch to the registered per-case builder;
3. return the resolved `ProductTimeSpecAnchor`.

## Checker Flow

The anchor checker dispatch entry point is:

- `anchor::check_Anchor_or_throw(...)`

It is implemented in:

- `AnchorRegistry.h`

Behavior:

1. validate the classification enum value;
2. dispatch to the registered per-case checker;
3. return `true` on success;
4. otherwise throw a check failure.

The checker infrastructure is present in the registry and in the case files.
It is intended to validate the resolved anchor against both the case semantics
and the originating normalized input.

## Current Cases

### ForecastAnalysis

File:

- `impl/ForecastAnalysis.h`

Matcher:

- `match_ForecastAnalysis_Anchor(...)`

The case matches when:

- `marsDate` is present;
- `marsHdate` is absent;
- `marsYear` is absent;
- `marsMonth` is absent.

`marsTime` is optional.

Builder:

- `build_ForecastAnalysis_Anchor(...)`

Construction rule:

- all three anchor members are built from the same source datetime:
  - MARS `date`
  - optional MARS `time`
  - default time `00:00:00` when `marsTime` is absent

So in this case:

```text
labelDateTime == initialConditionsDateTime == referenceDateTime
```

Checker:

- `check_ForecastAnalysis_Anchor(...)`

The checker validates:

- anchor type is `ForecastAnalysis`;
- the three anchor datetimes are equal;
- the resolved date matches input `marsDate`;
- the resolved time matches input `marsTime` or the default time.

### Hindcast

File:

- `impl/Hindcast.h`

Matcher:

- `match_Hindcast_Anchor(...)`

The case matches when:

- `marsDate` is present;
- `marsHdate` is present;
- `marsYear` is absent;
- `marsMonth` is absent.

`marsTime` is optional.

Builder:

- `build_Hindcast_Anchor(...)`

Construction rule:

- `labelDateTime` is built from `marsHdate` at `00:00:00`;
- `initialConditionsDateTime` is built from MARS `date` and optional MARS
  `time`;
- `referenceDateTime` equals `initialConditionsDateTime`.

So in this case:

```text
labelDateTime = hdate @ 00:00:00
initialConditionsDateTime = date + optional time
referenceDateTime = initialConditionsDateTime
```

Checker:

- `check_Hindcast_Anchor(...)`

The checker validates:

- anchor type is `Hindcast`;
- `initialConditionsDateTime == referenceDateTime`;
- `labelDateTime <= initialConditionsDateTime`;
- `labelDateTime.date()` matches `marsHdate`;
- `labelDateTime.time()` is `00:00:00`;
- `initialConditionsDateTime` matches MARS `date` and optional MARS `time`.

### SeasonalClimate

File:

- `impl/SeasonalClimate.h`

Matcher:

- `match_SeasonalClimate_Anchor(...)`

This case is currently reserved but intentionally not implemented.

Current behavior:

- if `marsYear` or `marsMonth` is present, the matcher throws a
  `not implemented` model exception;
- otherwise it returns `false`.

Builder:

- `build_SeasonalClimate_Anchor(...)`

Current behavior:

- always throws a `not implemented` model exception.

Checker:

- `check_SeasonalClimate_Anchor(...)`

Current behavior:

- always throws a `not implemented` model exception.

This means the case remains visible in the registry and classification model,
but is intentionally blocked until real implementation work is done.

## Registry Responsibilities

`AnchorRegistry.h` owns the immutable case table.

Each row stores:

- classification value;
- diagnostic name;
- matcher callback;
- builder callback;
- checker callback.

This keeps the three anchor callback families aligned for each case.

## Exception Layering

The current anchor layer uses two levels of exception responsibility.

Low-level invariant helper:

- `checkedAnchor(...)`
- throws `Mars2GribGenericException`
- does not know about model input or JSON context

Case-level callbacks:

- matcher
- builder
- checker
- catch failures and rethrow as `Mars2GribModelException`
- attach `input.to_json()` for richer debug context

This keeps low-level invariant code small and reusable while preserving rich
diagnostics at the callback boundary.

## Diagnostic JSON

`AnchorDataTypes.h` provides:

- `productTimeSpecAnchorJson(...)`

This function is:

- best-effort;
- `noexcept`;
- intended only for diagnostic context construction.

If serialization fails, it returns a stable fallback JSON error object instead
of throwing.

## Current Limitations

The main current limitation of the anchor folder is:

- `SeasonalClimate` is recognized as a reserved case but intentionally not
  implemented.

The anchor checker infrastructure is present and the registry now exposes the
checker dispatch entry point.

Top-level normalization and cross-artifact final checks still remain later
pipeline stages outside the anchor folder.
