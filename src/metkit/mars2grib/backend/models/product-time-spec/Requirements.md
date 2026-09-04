<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# ProductTimeSpec Requirements

## Purpose

This file is the authoritative task tree for the rewrite and hardening of
`mars2grib/backend/models/product-time-spec`.

This file is intentionally operational. It must be sufficient to restart the
work later without relying on previous chat history.

This file is organized as follows:

- one tree-style task list near the top;
- one detailed section for every leaf task in the tree;
- when one task is fully completed, remove its tree item and remove its detailed
  section.

## Scope

This file applies to:

- `src/metkit/mars2grib/backend/models/product-time-spec/`
- `src/metkit/mars2grib/utils/TemporalArithmetic.h`
- related option plumbing under:
  - `src/metkit/mars2grib/api/`
  - `src/metkit/mars2grib/utils/dictionary_traits/`

## Task Tree

- `5.0` Add raw-to-normalized artifact layer.
  Normalize callback outputs only after raw build and raw checks succeed.
- `5.1` Normalize time ranges to hours.
  Convert all final ranges to hours using exact placed intervals where monthly
  semantics are involved.
- `5.2` Normalize time increments to seconds or missing.
  Convert all final increments to seconds or missing only.
- `5.3` Validate normalized sub-monthly time ranges.
  Enforce the language-derived hour whitelist.
- `5.4` Validate normalized monthly time ranges.
  Enforce the ecCodes-derived month-hour whitelist.
- `6.0` Add cross anchor/domain/shape checks.
  Validate consistency only after normalization has completed.
- `6.1` Validate domain offsets against reference.
  Ensure hour offsets exactly match the real datetime differences.
- `6.2` Validate shape/domain span relationships.
  Ensure outer and inner windows remain consistent with domain placement.
- `6.3` Validate synoptic cross semantics.
  Ensure synoptic anchor, domain, and shape facts agree.
- `6.4` Validate seasonal cross semantics.
  Ensure month-based raw semantics normalize consistently.
- `7.0` Audit diagnostic JSON.
  Make all diagnostic JSON helpers best-effort and `noexcept`.
- `7.1` Audit nontrivial function exception boundaries.
  Ensure full-body `try/catch` with nested rethrow everywhere required.
- `7.2` Add failure-only templated matcher diagnostics.
  Keep matcher logic single-sourced while allowing the failure path to rerun
  matchers in debug mode and collect the composed result plus all named
  conditions.
- `8.0` Harden `TemporalArithmetic`.
  Review logic, precision, overflow, and whole-hour/whole-second safety.
- `8.1` Add exact whole-hour helpers.
  Provide the helpers needed by normalization and domain-offset validation.
- `8.2` Harden month-based arithmetic.
  Make month handling safe, explicit, and suitable for exact hour derivation.
- `9.0` Rewrite code documentation.
  Update file headers and function documentation after the implementation is
  stabilized.

## Task Details

Tasks `0.x` through `4.x` are implemented in code. The remaining active work in
this file begins at the raw-to-normalized artifact layer.

The normalization stage currently targets only the final shape windows. The
domain artifact remains unchanged and supplies the real placement needed for
month-based range normalization.

### `0.0` Extend normalized input with precomputed step-zero statistical-processing allowance

Before the callback-by-callback rewrite begins, extend the normalized input
contract with a precomputed boolean describing whether the innermost
`TypeOfStatisticalProcessing` is allowed in the step-zero from-start special
case.

This is intentionally an early task because it changes the input contract and
simplifies later callback reviews.

This rule is considered an input policy fact rather than shape-construction
logic and is therefore allowed to live in `ProductTimeSpecInput`.

### `0.1` Update `isAllowed_InnerTypeOfStatisticalProcessingAtStepZero` semantics

The helper currently in `shapes/ShapeUtils.h` must be reviewed and its
semantics adjusted.

Required change:

- if `innerTypeOfStatisticalProcessing` is
  `TypeOfStatisticalProcessing::Missing`, the result must be `true`.

Reason:

- `Missing` means the instant case;
- the instant case is always valid for this allowance test;
- keeping that rule explicit in the helper makes the later derived input member
  semantically correct.

### `0.2` Add precomputed step-zero allowance to `ProductTimeSpecInput`

Add a new boolean member to `ProductTimeSpecInput` representing the result of
the step-zero statistical-processing allowance rule.

Preferred semantic meaning:

- this member should express whether the innermost statistical processing is
  allowed in the zero-step from-start special case.

The purpose of this member is:

- remove repeated local recomputation in callbacks;
- make the input contract richer and more explicit;
- improve readability during callback review.

### `0.3` Compute the new input member during `make_ProductTimeSpecInput_or_throw`

The new boolean member must be computed during normalized input assembly.

The computation should use:

- the resolved `innerMostTypeOfStatisticalProcessing`;
- `allowExtendedSetOfOperationsForZeroLengthFsWindow`;
- the reviewed helper semantics from task `0.1`.

This derived fact should be available to later callbacks without requiring
callback-local policy recomputation.

### `0.4` Update zero-step shape callbacks to consume the input member

After the normalized input member exists, the step-zero shape callbacks should
consume that member instead of recomputing the allowance locally.

This applies at minimum to the current zero-step from-start shape matchers.

The goal is:

- simpler callback logic;
- fewer hidden policy decisions in callbacks;
- better callback reviewability.

### `1.0` Extend raw domain artifact

Update `ProductTimeSpecDomain` while keeping the existing absolute fields.

The raw domain artifact must retain:

- `domainStartDateTime`
- `domainEndDateTime`

The raw domain artifact must add:

- `bool isSynoptic`
- `long startOffsetHoursFromReference`
- `long endOffsetHoursFromReference`

The purpose of the new fields is:

- explicit representation of synoptic semantics;
- easier comparison and reconstruction of domains;
- direct availability of encoder-facing hour offsets;
- support for exact month-based normalization after callback execution.

This task changes the raw model contract first. It must be done before the full
callback rewrite because many later tasks depend on the extended raw domain
representation.

### `1.1` Review `match_Analysis_Domain`

Review the matcher line by line.

Verify:

- whether the case boundary is complete;
- whether it overlaps with any other domain matcher;
- whether helper logic should be pulled into the callback for clarity;
- whether the comments still match the implementation.

No blind edit is allowed.

### `1.2` Review `build_Analysis_Domain`

Rework the builder so it becomes leaf logic.

The final callback should visibly compute:

- start datetime;
- end datetime;
- `isSynoptic`;
- start offset from reference in hours;
- end offset from reference in hours.

The builder should not hide semantic control flow in nested helpers except for
`TemporalArithmetic` and trivial low-level primitives.

### `1.3` Review `match_Forecast_Domain`

Review the matcher line by line.

Verify:

- the non-synoptic condition;
- the non-seasonal condition;
- the forecast condition;
- the non-from-start condition;
- exclusivity versus the other forecast-domain matchers.

### `1.4` Review `build_Forecast_Domain`

Rework the builder so it visibly computes all raw domain members.

The domain logic should remain readable and explicit.

### `1.5` Review `match_FromStartForecast_Domain`

Review the matcher line by line.

Verify:

- the from-start condition;
- exclusivity versus normal forecast and seasonal forecast domain cases;
- whether helper logic should be local.

### `1.6` Review `build_FromStartForecast_Domain`

Rework the builder so it visibly computes the raw from-start domain and all raw
domain members.

### `1.7` Review `match_SeasonalForecast_Domain`

Review the matcher line by line.

Verify:

- the seasonal discriminator use;
- the non-synoptic condition;
- the forecast condition;
- exclusivity versus the other forecast-domain cases.

### `1.8` Review `build_SeasonalForecast_Domain`

Rework the builder so it visibly computes the raw month-based domain and all raw
domain members.

Month semantics must remain raw here and must not be prematurely normalized.

### `1.9` Review `match_SynopticAnalysis_Domain`

Review the matcher line by line.

Verify:

- explicit synoptic condition;
- IFS restriction;
- analysis restriction;
- exclusivity versus all non-synoptic domain cases.

### `1.10` Review `build_SynopticAnalysis_Domain`

Rework the builder so it visibly computes the raw synoptic domain and all raw
domain members.

The implementation should preserve the real placed datetimes while making the
synoptic nature explicit in `isSynoptic` and the reference-relative hour
offsets.

### `2.0` Add per-anchor check callbacks

Introduce one check callback per anchor case and dispatch them immediately after
raw anchor construction.

These checks are raw-output checks, not normalization checks.

They should validate:

- required field presence;
- ordering invariants;
- case-specific anchor semantics.

### `2.1` Review `match_ForecastAnalysis_Anchor`

Review the matcher line by line for correctness, exclusivity, and readability.

### `2.2` Review `build_ForecastAnalysis_Anchor`

Rework the builder so it visibly computes each anchor field locally and clearly.

### `2.3` Review `match_Hindcast_Anchor`

Review the matcher line by line for correctness, exclusivity, and readability.

### `2.4` Review `build_Hindcast_Anchor`

Rework the builder so it visibly computes each anchor field locally and clearly.

### `2.5` Review `match_SeasonalClimate_Anchor`

Review the matcher line by line for correctness, exclusivity, and readability.

### `2.6` Review `build_SeasonalClimate_Anchor`

Rework the builder so it visibly computes each anchor field locally and clearly.

### `3.0` Add per-domain check callbacks

Introduce one check callback per domain case and dispatch them immediately after
raw domain construction.

These checks should validate:

- ordering;
- synoptic flag correctness;
- offset correctness at the raw level when possible;
- case-specific domain semantics.

### `4.0` Add per-shape check callbacks

Introduce one check callback per shape case and dispatch them immediately after
raw shape construction.

These checks should validate:

- expected number of windows;
- required presence or absence of raw values;
- raw ordering;
- case-specific semantic invariants.

### `4.1` to `4.42` Review shape callbacks one by one

Each shape callback task listed in the tree is intentionally separate because no
shape callback may be modified blindly.

For every shape matcher review:

- verify case boundaries;
- verify exclusivity;
- verify comments versus implementation;
- decide whether helper logic must become local.

For every shape outer-range builder review:

- verify what is raw and what must remain deferred;
- keep month semantics raw where needed;
- keep the logic visible and local.

For every shape window builder review, enforce this mandatory structure:

1. input validation;
2. one compact documented block of code for each member of
   `ProductTimeSpecWindow`;
3. members validation;
4. final construction of the window from already assigned local variables.

When more than one window is built, apply the same pattern inside a loop.

Every per-member compact block must be documented with an explanation of why
that field is computed that way for that specific case.

This is a hard readability and reviewability requirement.

### `5.0` Add raw-to-normalized artifact layer

Introduce a clear stage that transforms raw callback outputs into normalized
artifacts.

Raw callback outputs are allowed to preserve natural case semantics.

Normalized artifacts must be encoder-facing and canonical.

This stage should run only after:

- raw anchor checks succeed;
- raw domain checks succeed;
- raw shape checks succeed.

Current implementation direction:

- normalize only `shape::ProductTimeSpecShape`;
- keep `domain::ProductTimeSpecDomain` unchanged;
- store only normalized windows in the final `ProductTimeSpec` object.

### `5.1` Normalize time ranges to hours

All final time ranges must be normalized to hours.

Sub-monthly ranges must be normalized to the allowed hour values.

Month-based ranges must be normalized using the exact placed interval and the
real month length in that exact year and month.

For month-based shape windows, the placed interval begins at the real domain
start:

- non-synoptic: `domain.domainStartDateTime`;
- synoptic: midnight of `domain.domainStartDateTime.date()`.

The outermost normalized window range must equal the real domain span in hours.

### `5.2` Normalize time increments to seconds or missing

All final time increments must be represented as either:

- seconds; or
- missing.

No final normalized increment should remain in hour, day, or month units.

Trivially convertible units should be converted directly to seconds.

Month-valued final increments are not supported and must be rejected.

### `5.3` Validate normalized sub-monthly time ranges

Validate normalized sub-monthly hour values against the language-derived source
of truth.

The currently mirrored allowed values visible in `deductions/timespan.h` are:

- `1`
- `3`
- `6`
- `12`
- `18`
- `24`
- `48`
- `72`
- `120`
- `168`
- `240`
- `360`

The source of truth remains the language definition.

### `5.4` Validate normalized monthly time ranges

Validate normalized month-based hour values against the ecCodes-derived allowed
set:

- `672`
- `696`
- `720`
- `744`

These values must be derived from the exact placed interval before validation.

### `6.0` Add cross anchor/domain/shape checks

Add a dedicated post-normalization validation stage that checks consistency
across the three categories.

These checks are cross-artifact checks and should not be mixed with the raw
callback checks.

### `6.1` Validate domain offsets against reference

Ensure that:

- `startOffsetHoursFromReference`
- `endOffsetHoursFromReference`

exactly match the corresponding datetime differences from
`anchor.referenceDateTime`.

### `6.2` Validate shape/domain span relationships

Ensure that the normalized shapes remain consistent with the normalized domain
placement and span.

This includes from-start and multi-window cases.

### `6.3` Validate synoptic cross semantics

Ensure that synoptic anchor, domain, and shape facts are mutually consistent.

### `6.4` Validate seasonal cross semantics

Ensure that month-based seasonal semantics normalize consistently and remain
valid across anchor, domain, and shape.

### `7.0` Audit diagnostic JSON

Every diagnostic JSON helper used in exception enrichment must become:

- best-effort;
- `noexcept`;
- stable under failure.

No diagnostic JSON function may throw while a higher-level function is handling
an exception.

### `7.1` Audit nontrivial function exception boundaries

Every nontrivial function should follow the full-body pattern:

```cpp
try {
    // possibly faulty code
}
catch (...) {
    std::throw_with_nested(...);
}
```

This applies to:

- callbacks;
- check callbacks;
- normalization helpers;
- cross-check helpers;
- `TemporalArithmetic`;
- any other nontrivial model logic.

### `7.2` Add failure-only templated matcher diagnostics

Classification failures caused by zero matches or overlapping matches are
currently hard to diagnose because the implementation usually exposes only the
final count and, in some places, only coarse matcher-level `true` or `false`
results.

Add a failure-only matcher diagnostic mechanism with the following properties:

- matcher logic must remain single-sourced;
- the template must remain visible in the code;
- the normal matcher instantiation must return `bool`;
- the debug matcher instantiation must return `std::string`;
- the debug string must contain:
  - the matcher name;
  - the final composed matcher result;
  - the full ordered set of named matcher conditions.

The intended design is one visible templated matcher per case, with two compile-
time instantiations.

The preferred direction is:

- `match_<Case>_<Category><MatcherMode::Normal>(input)` for the standard path;
- `match_<Case>_<Category><MatcherMode::Debug>(input)` for the failure-only
  diagnostic path.

This mechanism is not a normal-path check.

It must never run in the hot path.

The normal workflow should use only the `bool` matcher instantiation.

Only when a classification failure already happened should the error-handling
path rerun the matchers in debug mode and collect the resulting strings to add
more context to the exception.

Because the two matcher instantiations have different return types, the case
registry structs should be extended so they can store both:

- the normal matcher function pointer;
- the debug matcher function pointer.

This applies consistently across:

- anchor registries;
- domain registries;
- shape registries.

The motivation for this requirement is:

- no duplication of matcher logic;
- no debug cost in the hot path;
- much richer diagnostics for zero-match and overlap failures;
- explicit and reviewable matcher semantics.

The failure-path debug collection must itself be best-effort.

If debug collection fails while enriching an existing exception, that secondary
failure must not hide the original failure.

### `8.0` Harden `TemporalArithmetic`

Review `TemporalArithmetic` for:

- overflow and underflow;
- month arithmetic correctness;
- whole-second assumptions;
- whole-hour support required by normalization;
- precision and robustness;
- readable failure modes.

### `8.1` Add exact whole-hour helpers

Add the helpers required by:

- domain offset computation;
- normalized hour-range validation;
- post-normalization cross-checking.

### `8.2` Harden month-based arithmetic

Ensure month-based date arithmetic is safe and suitable for deriving exact hour
lengths for concrete placed intervals.

### `9.0` Rewrite code documentation

After the code has stabilized:

- update file headers;
- update function documentation;
- update data-structure documentation;
- update licenses if needed for consistency.

Markdown rewrite beyond this file and `Status.md` should only happen after the
implementation is stabilized.
