# ProductTimeSpec Modification Plan

**Repository:** `ecmwf/metkit`  
**Branch reviewed:** `feature/new-product-time-spec`  
**Primary subtree:** `src/metkit/mars2grib/backend/models/product-time-spec`  
**Review date:** 2026-07-23  
**Purpose:** provide a complete, self-contained implementation specification for the agreed corrections and additions.

---

## 1. Scope and reading instructions

This document describes the changes required in and around the new backend model:

```text
src/metkit/mars2grib/backend/models/product-time-spec
```

It also includes the directly affected option API, dictionary traits, deductions documentation, utilities, and tests.

The instructions are written so that an engineer unfamiliar with the implementation can apply the modifications without needing the preceding design discussion.

### Priority meanings

| Priority | Meaning |
|---|---|
| **Critical** | Required to prevent incorrect temporal semantics or invalid canonical models. Must be completed before delivery. |
| **High** | Required by the agreed API/behaviour contract or necessary to make critical checks configurable and testable. Complete for this delivery unless an explicit blocker is found. |
| **Medium** | Local robustness, documentation, and maintainability work with low implementation risk. Complete after Critical and High items. |
| **Low** | Architectural improvements that are desirable but explicitly deferred because they are too invasive for the current late delivery. |

---

## 2. Non-negotiable decisions

The following decisions are final for this delivery.

### 2.1 Correct anchor semantics

When both `date/time` and `hdate` are present:

```text
labelDateTime             = hdate at 00:00:00
initialConditionsDateTime = date/time
referenceDateTime         = initialConditionsDateTime
```

More generally:

```text
labelDateTime =
    hindcastDateTime
    else dateTime
    else yearMonthDateTime

initialConditionsDateTime =
    dateTime
    else labelDateTime

referenceDateTime =
    yearMonthDateTime
    else initialConditionsDateTime
```

The required ordering remains:

```text
labelDateTime <= initialConditionsDateTime <= referenceDateTime
```

The current anchor-building code already follows this mapping. The required correction is primarily to stale deduction documentation that still describes `dateTime` as the direct label source.

### 2.2 Statistical support before `referenceDateTime`

For this delivery, retain the global rule:

```text
domainStartDateTime >= referenceDateTime
```

Backward-looking, synoptic, or other exceptional products will be supported later through an explicit whitelist. Do not add the whitelist now.

### 2.3 Duration type

Use:

```cpp
metkit::mars2grib::backend::deductions::TimeDuration
```

throughout the new ProductTimeSpec implementation.

Do not introduce a model-local duplicate duration type.

The normalized input must continue to represent an optional explicit increment as:

```cpp
std::optional<deductions::TimeDuration> timeIncrement;
```

There is no problem with `std::optional`; do not replace it with a separate presence boolean.

### 2.4 Explicit time increment deduction

Leave the new deduction in:

```text
src/metkit/mars2grib/backend/deductions/timeIncrement.h
```

unchanged if it works.

In particular, an explicit `timeIncrementInSeconds` that is an exact number of hours may continue to be canonicalized as `TimeUnit::Hour`.

Do not inspect, modify, reuse, or extend the legacy:

```text
src/metkit/mars2grib/backend/deductions/timeIncrementInSeconds.h
```

That path exists only for backward compatibility and will eventually be removed.

### 2.5 ProductTimeSpec value semantics

Keep the private `ProductTimeSpec` members `const`.

The object is deliberately immutable and non-assignable. Do not remove `const` from:

```cpp
const TimeAnchorKind anchorType_;
const ProductTimeSpecShapeKind shapeType_;
const TimeIncrementKind incrementType_;
const ProductTimeSpecAnchor anchor_;
const ProductTimeSpecDomain domain_;
const ProductTimeSpecWindows windows_;
```

### 2.6 Templates

Keep the current model functions templated.

Do not convert the post-normalization functions to fixed `ProductTimeSpecInput` prototypes or move them into non-template `.cc` implementations.

A future cross-cutting development will introduce a templated `Context_t` throughout `mars2grib`, likely resembling:

```cpp
template <typename Input_t, typename Context_t>
```

The context will cache `ProductTimeSpec` so concepts do not reconstruct it repeatedly. This future change is expected to affect essentially every function in `mars2grib`; it is not part of this delivery.

### 2.7 Domain construction

Keep domain construction independent from window construction.

The domain builder must continue selecting the outer range directly from normalized input. Final consistency validation must independently verify the result using the canonical windows. This redundancy is intentional and acts as a double check.

### 2.8 Instant representation

Keep the existing one-window instant sentinel:

```text
typeOfStatisticalProcessing = Missing
timeRange                    = 0 seconds
timeIncrement                = 0 seconds
```

Do not replace it with an empty window collection or a variant-based representation.

### 2.9 AIFS missing increment predicate

The current predicate is sufficient:

```text
marsClass == "ml" && numberOfRealWindows == 1
```

Do not add `type`, `stream`, or another product-classification key to this predicate.

### 2.10 Nested diagnostics

Preserve the existing nested exception and diagnostic structure exactly.

Do not reduce wrapping, flatten exceptions, replace model exceptions with assertions, or remove repeated `input.to_json()` context.

All new consistency failures must use the same consistency-layer exception type and nesting mechanism as the existing checks.

### 2.11 Internal component visibility

The ideal long-term public surface is `ProductTimeSpec` only, with anchor/domain/window component types treated as implementation details.

Do not perform that encapsulation refactor in this delivery. It is too invasive and not a delivery blocker.

---

# 3. Priority summary

## Critical

1. Correct the inverted `dateTime` documentation.
2. Add final calendar-aware window/domain consistency checks.
3. Implement the exact zero-length from-start exception semantics.
4. Preserve independent domain construction and validate the outer range using the canonical window.
5. Add mandatory tests for all new invariants.

## High

1. Add the `requireExactWindowDivisibility` option, defaulting to `false`.
2. Integrate the option through all active option input mechanisms and dictionary traits.
3. Add calendar-aware exact divisibility for every adjacent pair of real windows.
4. Materialize automatically defaulted increments as seconds.
5. Add strong warnings for unresolved `FakeSingleLoopDoubleLoop` and `typeOfTimeIncrement` logic.
6. Add overflow-safe calendar-month arithmetic.

## Medium

1. Make every ProductTimeSpec header self-contained.
2. Harden diagnostic JSON string escaping for all control characters below `0x20`.
3. Update documentation tables and stage descriptions for the new checks and option.
4. Correct stale Doxygen grouping where ProductTimeSpec model headers are still described as deductions.

## Low / deferred

1. Hide all ProductTimeSpec component structures from external users.
2. Introduce the future `Context_t` cache.
3. Add the whitelist for backward-looking or synoptic statistics.
4. Remove the legacy time-increment path.
5. Replace sequential divisibility evaluation with a more optimized calendar-duration algebra, if ever needed.
6. Refactor repeated options parsing into a single declarative option registry.
7. Alter `ProductTimeSpec` copy/move assignment semantics.
8. Reduce nested diagnostics.

---

# 4. Critical modifications

## C1. Correct the `dateTime` semantic documentation

### Problem

The current code builds the anchor correctly, but the public deduction documentation in:

```text
src/metkit/mars2grib/backend/deductions/dateTime.h
```

still calls `dateTime` a direct **label** datetime source.

That is incorrect.

`dateTime` is the direct **initial-conditions** datetime source. `hindcastDateTime`, derived from `hdate`, is the direct hindcast label source.

### Files to modify

```text
src/metkit/mars2grib/backend/deductions/dateTime.h
src/metkit/mars2grib/backend/deductions/hindcastDateTime.h
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchor.h
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecAnchorClassification.h
src/metkit/mars2grib/product-time-spec/productTimeSpecV3_final.md
```

The model headers currently appear consistent, but they must be reviewed so no stale wording remains.

### Required wording changes in `dateTime.h`

Replace every use of:

```text
direct label datetime source
```

with:

```text
direct initial-conditions datetime source
```

Update the `@return`, `@throws`, wrapper documentation, and summary text accordingly.

The deduction still performs exactly the same parsing:

```text
date absent, time absent -> nullopt
date present, time absent -> DateTime(date, 00:00:00)
date present, time present -> DateTime(date, time)
time present without date -> error
```

Do not change parsing or runtime behaviour.

### Required wording in `hindcastDateTime.h`

Make the semantic role explicit:

```text
hindcastDateTime is the direct labelDateTime source when hdate is present
```

The deduction remains:

```text
hdate -> DateTime(hdate, 00:00:00)
```

### Required anchor truth table

Include the following table in either `ProductTimeSpecAnchor.h` or the main specification document, and ensure all code documentation agrees with it.

| `dateTime` | `hindcastDateTime` | `yearMonthDateTime` | `labelDateTime` | `initialConditionsDateTime` | `referenceDateTime` |
|---|---|---|---|---|---|
| present | absent | absent | `dateTime` | `dateTime` | `dateTime` |
| present | present | absent | `hindcastDateTime` | `dateTime` | `dateTime` |
| present | absent | present | `dateTime` | `dateTime` | `yearMonthDateTime` |
| present | present | present | `hindcastDateTime` | `dateTime` | `yearMonthDateTime` |
| absent | present | absent | `hindcastDateTime` | `hindcastDateTime` | `hindcastDateTime` |
| absent | absent | present | `yearMonthDateTime` | `yearMonthDateTime` | `yearMonthDateTime` |
| absent | present | present | `hindcastDateTime` | `hindcastDateTime` | `yearMonthDateTime` |
| absent | absent | absent | invalid | invalid | invalid |

After materialization, always enforce:

```text
labelDateTime <= initialConditionsDateTime <= referenceDateTime
```

### Repository-wide verification

Run a text search over the new implementation and documentation:

```bash
grep -RIn \
    -e 'dateTime.*label' \
    -e 'direct label datetime' \
    -e 'label datetime source' \
    src/metkit/mars2grib/backend/deductions \
    src/metkit/mars2grib/backend/models/product-time-spec \
    src/metkit/mars2grib/product-time-spec
```

Every match must be inspected. Do not mechanically replace occurrences where `dateTime` is used as a fallback label when no `hdate` exists; that fallback is valid. The incorrect statement is that `dateTime` is intrinsically the direct label source.

---

## C2. Add final calendar-aware consistency validation

### Purpose

Duration comparisons cannot be performed safely by comparing:

```cpp
duration.length
duration.unit
```

or by converting every duration to seconds.

`Month` is calendar-dependent, and `Day` has explicitly calendar-aligned semantics in this implementation.

All range ordering, increment ordering, and divisibility checks described below must therefore be evaluated by applying durations to the same absolute anchor:

```text
domain.domainStartDateTime
```

### Files to modify

```text
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecConsistency.h
src/metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecConsistency_details.h
src/metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecTimeUtils.h
```

### Real-window definition

The instant sentinel is not a real statistical window.

Use the existing rule:

```cpp
nRealWindows =
    shapeType == ProductTimeSpecShapeKind::Instant
        ? 0
        : windows.values.size();
```

All checks in this section apply only to real windows.

### Required validation order

After the existing anchor, domain-ordering, and window-cardinality checks:

1. Handle the instant shape using the existing instant sentinel validation and return.
2. Validate the outer-window/domain equality.
3. Detect the explicitly allowed zero-length from-start exception.
4. If that exception is active, validate its dedicated invariants and skip the three ordinary window checks.
5. Otherwise:
   - validate range versus increment for every window with a semantic increment;
   - validate adjacent range hierarchy;
   - optionally validate exact adjacent divisibility.
6. Continue with existing from-start, AIFS, and fake representation checks.

### New final-stage helper structure

Add helpers with narrow responsibilities. Suggested names:

```cpp
checkProductTimeSpecConsistencyOutermostDomainEquality_or_throw
isProductTimeSpecZeroLengthFromStartException
checkProductTimeSpecConsistencyZeroLengthFromStart_or_throw
checkProductTimeSpecConsistencyWindowIncrementBounds_or_throw
checkProductTimeSpecConsistencyWindowHierarchy_or_throw
checkProductTimeSpecConsistencyWindowDivisibility_or_throw
```

Names may be adjusted to match local style, but responsibilities must remain separate so each failure has a precise diagnostic.

---

## C3. Outer-window/domain equality

### Required invariant

For every non-instant product:

```text
domainStartDateTime + outerWindow.timeRange == domainEndDateTime
```

where:

```cpp
const auto& outerWindow = windows.values.front();
```

Evaluate it using:

```cpp
addProductTimeSpecDuration_or_throw(
    domain.domainStartDateTime,
    outerWindow.timeRange)
```

### Why addition must be used here

Domain construction already selects its outer range independently from input and generally computes the start by subtracting the range from the end.

The final check must use the canonical outer window and the opposite direction:

```text
domain construction:
    input outer range
    domainEnd - range -> domainStart

final validation:
    canonical outer window
    domainStart + range -> domainEnd
```

This is the intended independent double check.

### Existing helper

The current `checkProductTimeSpecConsistencyOutermostSupport_or_throw` recomputes:

```text
domainEnd - windows[0].timeRange
```

and compares the result with `domainStart`.

Replace that arithmetic with the required forward equality, or add a new forward helper and remove the duplicate subtraction check. The final stage should not simply repeat the same subtraction direction already used during domain construction.

Retain the existing global check:

```text
domainStartDateTime >= anchor.referenceDateTime
```

for this delivery.

### Failure diagnostic

Use a `Mars2GribModelException` message that identifies both artifacts, for example:

```text
Final ProductTimeSpec validation detected that
`domainStartDateTime + outerWindow.timeRange != domainEndDateTime`
```

Attach `input.to_json()` and `Here()` exactly like existing consistency errors.

---

## C4. Window increment bound

### Ordinary invariant

For every real window carrying a semantic increment:

```text
domainStartDateTime + window.timeRange
    >=
domainStartDateTime + window.timeIncrement
```

This is the calendar-aware form of:

```text
timeIncrement <= timeRange
```

### Semantic increment definition

Perform this check only when the increment is real:

```cpp
incrementType == TimeIncrementKind::ExplicitIncrement
    ||
incrementType == TimeIncrementKind::DefaultedIncrement
```

Skip it for:

```cpp
TimeIncrementKind::AifsPureMissingIncrement
```

because the stored zero duration is a missing-increment sentinel, not a real increment.

`NoIncrement` is expected for instant products and is already handled by the instant branch.

### Algorithm

For each real window:

```cpp
const eckit::DateTime rangeEnd =
    addProductTimeSpecDuration_or_throw(
        domain.domainStartDateTime,
        window.timeRange);

const eckit::DateTime incrementEnd =
    addProductTimeSpecDuration_or_throw(
        domain.domainStartDateTime,
        window.timeIncrement);

if (rangeEnd < incrementEnd) {
    throw Mars2GribModelException(...);
}
```

All standard exceptions from calendar arithmetic must be wrapped in the existing consistency-layer `Mars2GribModelException`, preserving nesting and `input.to_json()`.

### Do not perform this in the windows builder

This check must not be placed in:

```text
productTimeSpecWindows_details.h
```

It requires the final domain anchor and calendar-aware arithmetic. It belongs in final consistency validation during ProductTimeSpec construction.

---

## C5. Adjacent window hierarchy

### Required invariant

For every adjacent pair of real windows, ordered outermost to innermost:

```text
domainStartDateTime + windows[i].timeRange
    >=
domainStartDateTime + windows[i + 1].timeRange
```

for:

```cpp
i = 0; i + 1 < nRealWindows; ++i
```

This is the mandatory calendar-aware form of:

```text
outerTimeRange >= innerTimeRange
```

### Algorithm

```cpp
for (std::size_t i = 0; i + 1 < nRealWindows; ++i) {
    const auto outerEnd =
        addProductTimeSpecDuration_or_throw(
            domain.domainStartDateTime,
            windows.values[i].timeRange);

    const auto innerEnd =
        addProductTimeSpecDuration_or_throw(
            domain.domainStartDateTime,
            windows.values[i + 1].timeRange);

    if (outerEnd < innerEnd) {
        throw Mars2GribModelException(...);
    }
}
```

### Placement

This check belongs only in the final consistency stage.

Do not add an early fixed-unit or `{length, unit}` comparison in the window builder.

---

## C6. Explicitly allowed zero-length from-start exception

### Activation condition

The exception is active only when all of the following are true:

```text
shapeType == FromStartSingleLoop
resolved step == 0
input.allowZeroLengthFsWindow == true
```

The existing `allowZeroLengthFsWindow` option controls the complete exception. Do not add another option.

A zero duration is determined by:

```cpp
duration.length == 0
```

regardless of duration unit.

### Required invariants

When the exception is active, final consistency must explicitly verify:

```text
domainStartDateTime == domainEndDateTime
outerWindow.timeRange.length == 0
domainStartDateTime + outerWindow.timeRange == domainEndDateTime
```

The third condition is the general outer-window/domain equality and must still be evaluated.

### Checks skipped by the exception

Skip all of the following:

1. range versus increment:
   ```text
   domainStart + timeRange[i] >= domainStart + timeIncrement[i]
   ```

2. adjacent window hierarchy:
   ```text
   domainStart + timeRange[i] >= domainStart + timeRange[i+1]
   ```

3. exact divisibility:
   ```text
   outerRange is an exact sequential multiple of innerRange
   ```

Do not skip anchor ordering, domain ordering, cardinality, outer equality, or the explicit zero-domain checks.

### Recommended helper

```cpp
template <typename Input_t>
bool isProductTimeSpecZeroLengthFromStartException(
    const Input_t& input,
    ProductTimeSpecShapeKind shapeType);
```

The helper should inspect the normalized optional `step`, not derive the condition from the stored window alone.

---

# 5. High-priority modifications

## H1. Add `requireExactWindowDivisibility`

### Behaviour

Add a new boolean option:

```cpp
bool requireExactWindowDivisibility{false};
```

The default must be `false` to preserve current behaviour.

When enabled, every adjacent pair of real windows must be exactly divisible using sequential calendar arithmetic.

The option is ignored for:

- instant products, because there are no real windows;
- the explicitly enabled zero-length from-start exception;
- single-real-window products, because there is no adjacent pair.

### Files to modify

At minimum:

```text
src/metkit/mars2grib/api/Options.h
src/metkit/mars2grib/api/Mars2Grib.cc
src/metkit/mars2grib/api/readOptionsFromLocalConfiguration.cc
src/metkit/mars2grib/api/readOptionsFromInitializerList.cc
src/metkit/mars2grib/utils/dictionary_traits/dictaccess_options.h
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecConsistency.h
src/metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecConsistency_details.h
```

Also update any local tests and documentation.

---

## H2. `Options.h`

### Default

In namespace `defaults`, add:

```cpp
inline constexpr bool requireExactWindowDivisibility = false;
```

### Member

Under the ProductTimeSpec policy section, add:

```cpp
///
/// @brief Require every adjacent pair of real ProductTimeSpec windows to be
/// exactly divisible.
///
/// When enabled, ProductTimeSpec validates each outer/inner adjacent range pair
/// using sequential calendar-aware arithmetic anchored at the resolved domain
/// start. This supports month-based ranges without converting them to a fixed
/// number of seconds.
///
/// The check is skipped for instant products, single-window products, and the
/// explicitly enabled zero-length from-start exception.
///
/// @default false
///
bool requireExactWindowDivisibility =
    defaults::requireExactWindowDivisibility;
```

Do not use a name other than:

```text
requireExactWindowDivisibility
```

---

## H3. Active LocalConfiguration parser

The active branch currently parses `eckit::LocalConfiguration` in:

```text
src/metkit/mars2grib/api/Mars2Grib.cc
```

Add:

```cpp
if (dict::has(conf, "requireExactWindowDivisibility")) {
    opts.requireExactWindowDivisibility =
        dict::get_or_throw<bool>(
            conf,
            "requireExactWindowDivisibility");
}
```

Match the actual generic dictionary API syntax used by the local code. The key must be a strict boolean; a present key with the wrong type must throw.

### Standalone parser mirror

The branch also contains:

```text
src/metkit/mars2grib/api/readOptionsFromLocalConfiguration.cc
```

Add the same key there if that file is retained. Even if it is currently not part of the active build, it must not drift further from `Mars2Grib.cc`.

---

## H4. Initializer-list parser

In:

```text
src/metkit/mars2grib/api/readOptionsFromInitializerList.cc
```

add:

```cpp
if (key == "requireExactWindowDivisibility") {
    opts.requireExactWindowDivisibility = readBool(key, value);
    return;
}
```

Unknown-key and duplicate-key behaviour must remain unchanged.

### Important branch-integration note

The reviewed branch contains a standalone initializer-list parser, but the reviewed `Mars2Grib.h` does not expose the corresponding constructor and the active `Mars2Grib.cc` does not contain that overload.

Before editing:

1. Check whether the local delivery branch has already integrated initializer-list options.
2. If integrated locally, add the new key to the active implementation.
3. If not integrated, update the standalone parser for consistency, but do not introduce an unrelated public-API expansion unless initializer-list support is part of the intended delivery.

The standalone parser currently contains stale references to `defaultTimeIncrementInSeconds`, while the reviewed `Options` aggregate does not define that member. If this file is brought into the active build, that inconsistency must be resolved first. Do not introduce a new configurable default increment as part of this task; the agreed defaulting logic remains inside ProductTimeSpec.

---

## H5. Options dictionary traits

Modify:

```text
src/metkit/mars2grib/utils/dictionary_traits/dictaccess_options.h
```

### `isBoolKey`

Add:

```cpp
key == "requireExactWindowDivisibility"
```

to `options_detail::isBoolKey`.

### `getBoolOrThrow`

Add:

```cpp
if (key == "requireExactWindowDivisibility") {
    return opts.requireExactWindowDivisibility;
}
```

### Required supported operations

The new key must work through:

```cpp
has(options, "requireExactWindowDivisibility")
has<bool>(options, "requireExactWindowDivisibility")
get_opt<bool>(options, "requireExactWindowDivisibility")
get_or_throw<bool>(options, "requireExactWindowDivisibility")
```

The options adapter remains read-only.

Do not add mutation, cloning, or generic dictionary serialization to these traits.

---

## H6. `ProductTimeSpecInput`

Modify:

```text
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecInput.h
```

### Add normalized policy field

```cpp
///
/// @brief Require exact calendar-aware divisibility between every adjacent
/// pair of real canonical windows.
///
bool requireExactWindowDivisibility{false};
```

### Populate it

In `make_ProductTimeSpecInput_or_throw`:

```cpp
input.requireExactWindowDivisibility =
    get_or_throw<bool>(
        opt,
        "requireExactWindowDivisibility");
```

Use the same typed access style as the other boolean policies.

### Diagnostic JSON

Add:

```json
"requireExactWindowDivisibility": true|false
```

to `ProductTimeSpecInput::to_json()`.

Place it with the other ProductTimeSpec policy fields.

### Options serialization

The reviewed branch does not expose an active general `Options::to_json()` serializer, and `dictaccess_options.h` explicitly says dictionary JSON serialization is unsupported.

Therefore:

- do not create a new general Options serializer solely for this option;
- do add the value to `ProductTimeSpecInput::to_json()`;
- if the local worktree contains another Options debug serializer not present in the reviewed branch, add the field there too.

---

## H7. Calendar-aware exact divisibility

### Scope

When:

```cpp
input.requireExactWindowDivisibility == true
```

and the zero-length from-start exception is not active, validate every adjacent pair of real windows.

For each `i`:

```text
outer = windows[i].timeRange
inner = windows[i + 1].timeRange
```

The outer range is divisible by the inner range if repeatedly adding the inner duration, starting at `domainStartDateTime`, lands exactly on the outer endpoint.

### Required sequential algorithm

```cpp
const eckit::DateTime outerEnd =
    addProductTimeSpecDuration_or_throw(
        domain.domainStartDateTime,
        outerRange);

eckit::DateTime current =
    domain.domainStartDateTime;

while (current < outerEnd) {
    const eckit::DateTime next =
        addProductTimeSpecDuration_or_throw(
            current,
            innerRange);

    if (next <= current) {
        throw Mars2GribModelException(
            "Calendar-aware window divisibility made no forward progress",
            input.to_json(),
            Here());
    }

    current = next;
}

if (current != outerEnd) {
    throw Mars2GribModelException(
        "Adjacent ProductTimeSpec window ranges are not exactly divisible",
        input.to_json(),
        Here());
}
```

### Important semantic requirement

Each addition uses the **evolving current datetime**:

```cpp
current = addDuration(current, innerRange);
```

Do not calculate:

```text
domainStart + n * innerRange
```

from the original anchor on every iteration.

Sequential addition is required because calendar-month arithmetic can depend on intermediate calendar states.

### Safety

The local window builder already requires positive real ranges, except for the explicitly enabled zero-length from-start case, which skips divisibility.

Still retain the `next <= current` guard. It prevents accidental infinite loops if a later change introduces a non-progressing duration.

Do not add an arbitrary iteration cap. Failure must be based on semantic non-progress or overshooting the outer endpoint.

---

## H8. Automatically defaulted increments must remain in seconds

Modify:

```text
src/metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecWindows_details.h
```

Only modify the `TimeIncrementKind::DefaultedIncrement` branch.

### Required outputs

```text
one-hour range  -> TimeDuration{600, Second}
larger range    -> TimeDuration{3600, Second}
day range       -> TimeDuration{3600, Second}
month range     -> TimeDuration{3600, Second}
```

### Exact code-level changes

Current hour-based branch:

```cpp
return innerRange.length == 1
    ? deductions::TimeDuration{600, tables::TimeUnit::Second}
    : deductions::TimeDuration{1, tables::TimeUnit::Hour};
```

Change the second result to:

```cpp
deductions::TimeDuration{
    3600,
    tables::TimeUnit::Second}
```

Current day/month branch:

```cpp
return deductions::TimeDuration{
    1,
    tables::TimeUnit::Hour};
```

Change it to:

```cpp
return deductions::TimeDuration{
    3600,
    tables::TimeUnit::Second};
```

The seconds branch already returns `600 s` or `3600 s` and can remain unchanged.

### Explicit increment path

Do not change:

```cpp
return *input.timeIncrement;
```

Explicit increments continue to use the canonicalization produced by `resolve_TimeIncrement_opt`.

---

## H9. Strong unresolved-logic disclaimers

### `FakeSingleLoopDoubleLoop`

Files:

```text
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecShapeClassification.h
src/metkit/mars2grib/backend/models/product-time-spec/detail/productTimeSpecShapeClassification_details.h
src/metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpecWindows.h
src/metkit/mars2grib/product-time-spec/productTimeSpecV3_final.md
```

Add a prominent Doxygen warning near the first public description:

```cpp
///
/// @warning
/// The identification policy for `FakeSingleLoopDoubleLoop` is not defined.
/// The current classifier intentionally returns `false` for every input.
/// Consequently, this representation is reserved but not operational.
/// Its `(type, class, paramId)` identification rules are still under
/// investigation and may change before the feature is considered complete.
///
```

Repeat a concise warning on:

```cpp
detail::is_FakeSingleLoopDoubleLoop
```

The helper must continue returning `false`.

Do not remove the enum value or builder branch. They are deliberate placeholders.

### `typeOfTimeIncrement`

File:

```text
src/metkit/mars2grib/backend/deductions/typeOfTimeIncrement.h
```

The current deduction already always throws. Strengthen the documentation with:

```cpp
///
/// @warning
/// The semantic mapping for GRIB `typeOfTimeIncrement` is still under
/// investigation. This deduction boundary is not operational and must not be
/// interpreted as a complete or stable mapping. The current implementation
/// always throws deliberately.
///
```

Do not add a fallback, default mapping, or partial heuristic in this task.

---

## H10. Overflow-safe calendar-month arithmetic

Modify:

```text
src/metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecTimeUtils.h
```

### Current unsafe expression

```cpp
const long total =
    year * 12L + monthIndex + months;
```

Signed overflow is undefined behaviour.

### Add checked helpers

Add checked signed multiplication and addition helpers appropriate for `long`.

Suggested interfaces:

```cpp
long checkedAddProductTimeSpecTime_or_throw(
    long lhs,
    long rhs,
    const char* context);

long checkedMultiplySignedProductTimeSpecTime_or_throw(
    long lhs,
    long rhs,
    const char* context);
```

The existing multiplication helper is specialized for non-negative durations. Do not silently reuse it for signed calendar years unless its contract is expanded and all callers remain correct.

### Safe calculation

Build `total` in explicit checked stages:

```cpp
const long yearMonths =
    checkedMultiplySignedProductTimeSpecTime_or_throw(
        year,
        12L,
        "calendar year-to-month conversion");

const long baseMonth =
    checkedAddProductTimeSpecTime_or_throw(
        yearMonths,
        monthIndex,
        "calendar base month index");

const long total =
    checkedAddProductTimeSpecTime_or_throw(
        baseMonth,
        months,
        "calendar month displacement");
```

Throw `std::overflow_error` with a precise message. Existing callers will wrap it into `Mars2GribModelException` with nested diagnostics.

Also review decrementing `newYear` after a negative remainder. Guard against underflow before:

```cpp
--newYear;
```

---

# 6. Medium-priority modifications

## M1. Make headers self-contained

Every public and detail header in:

```text
src/metkit/mars2grib/backend/models/product-time-spec
```

must compile when included as the first project header in an otherwise minimal translation unit.

Do not rely on transitive includes for:

```text
<cstddef>
<exception>
<limits>
<optional>
<sstream>
<stdexcept>
<string>
<string_view>
<utility>
<vector>
```

Only include the headers each file directly uses.

### Verification method

For each header, generate a minimal translation unit:

```cpp
#include "metkit/mars2grib/backend/models/product-time-spec/<Header>.h"

int main() {
    return 0;
}
```

Compile it with the normal project include paths.

Repeat for the detail headers if they are intended to be independently includable.

At minimum, inspect these known usages:

| Facility | Required header |
|---|---|
| `std::size_t` | `<cstddef>` |
| `std::throw_with_nested` | `<exception>` |
| `std::numeric_limits` | `<limits>` |
| `std::optional` | `<optional>` |
| `std::ostringstream` | `<sstream>` |
| `std::invalid_argument`, `std::overflow_error` | `<stdexcept>` |
| `std::string` | `<string>` |
| `std::move` | `<utility>` |
| `std::vector` | `<vector>` |

This is a compile-hygiene change only. Do not alter APIs or semantics while fixing includes.

---

## M2. Harden JSON escaping

Modify:

```text
src/metkit/mars2grib/backend/models/product-time-spec/detail/ProductTimeSpecJsonUtils.h
```

The helper:

```cpp
jsonQuote_modelInput
```

currently escapes:

```text
\
"
newline
carriage return
tab
```

It must also produce valid JSON for every control byte below `0x20`.

### Required behaviour

Use the named escapes where available:

```text
\b
\f
\n
\r
\t
```

For every other byte `c < 0x20`, emit:

```text
\u00XX
```

with uppercase or lowercase hexadecimal used consistently.

### Suggested implementation shape

```cpp
case '\b':
    out << "\\b";
    break;
case '\f':
    out << "\\f";
    break;
default:
    if (c < 0x20) {
        // emit \u00XX
    }
    else {
        out << static_cast<char>(c);
    }
    break;
```

Include the required formatting headers directly, for example `<iomanip>` if `std::hex`, `std::setw`, and `std::setfill` are used.

The function remains best-effort as part of the surrounding `to_json()` calls.

---

## M3. Update consistency documentation

Update the table and stage description in:

```text
ProductTimeSpecConsistency.h
```

Add explicit concerns for:

| Concern | Required check |
|---|---|
| outer support equality | `domainStart + outerRange == domainEnd` |
| increment bound | for each semantic increment, `domainStart + range >= domainStart + increment` |
| window hierarchy | for adjacent real windows, `domainStart + outerRange >= domainStart + innerRange` |
| optional divisibility | sequential calendar increments must land exactly on the outer endpoint |
| zero-length from-start | start equals end and outer range is zero; ordinary hierarchy checks skipped |

Make clear that all these checks are calendar-aware.

Update `ProductTimeSpecWindows.h` to state that local validation checks only local positivity and processing validity. Calendar-relative relationships are intentionally deferred to final consistency.

---

## M4. Doxygen groups

Several model headers still use:

```cpp
@ingroup mars2grib_backend_deductions
```

Review whether there is already a models-specific group.

If an appropriate group exists, use it consistently.

If no models-specific group exists, either:

1. create one in the relevant Doxygen grouping file; or
2. leave the current group unchanged for this delivery.

Do not create a broad documentation reorganization. The goal is to avoid incorrectly presenting the model layer as deduction logic.

---

# 7. Exact final consistency flow

The public function in:

```text
ProductTimeSpecConsistency.h
```

should have the following conceptual order.

```cpp
template <typename Input_t>
void validate_ProductTimeSpecConsistency_or_throw(
    const Input_t& input,
    TimeAnchorKind anchorType,
    ProductTimeSpecShapeKind shapeType,
    TimeIncrementKind incrementType,
    const ProductTimeSpecAnchor& anchor,
    const ProductTimeSpecDomain& domain,
    const ProductTimeSpecWindows& windows) {

    try {
        checkAnchor(...);
        checkDomainOrdering(...);
        checkWindowCardinality(...);

        if (shapeType == ProductTimeSpecShapeKind::Instant) {
            checkInstant(...);
            return;
        }

        checkOutermostDomainEquality(...);
        checkSupportNotBeforeReference(...);

        const bool zeroLengthFromStart =
            isZeroLengthFromStartException(
                input,
                shapeType);

        if (zeroLengthFromStart) {
            checkZeroLengthFromStart(...);
        }
        else {
            checkWindowIncrementBounds(
                input,
                incrementType,
                domain,
                windows);

            checkWindowHierarchy(
                input,
                domain,
                windows);

            if (input.requireExactWindowDivisibility) {
                checkWindowDivisibility(
                    input,
                    domain,
                    windows);
            }
        }

        if (shapeType ==
            ProductTimeSpecShapeKind::FromStartSingleLoop) {
            checkFromStart(...);
        }

        if (incrementType ==
            TimeIncrementKind::AifsPureMissingIncrement) {
            checkAifsMissingIncrement(...);
        }

        if (shapeType ==
            ProductTimeSpecShapeKind::FakeSingleLoopDoubleLoop) {
            checkFakeSingleLoopDoubleLoop(...);
        }
    }
    catch (...) {
        std::throw_with_nested(
            Mars2GribModelException(
                "Failed to validate `ProductTimeSpec` final consistency "
                "from normalized input",
                input.to_json(),
                Here()));
    }
}
```

The exact helper names may follow current naming conventions. The ordering and semantics must not change.

---

# 8. Tests

## 8.1 Test location

Add a dedicated test source:

```text
tests/mars2grib/backend/test_product_time_spec.cc
```

Register it in:

```text
tests/mars2grib/backend/CMakeLists.txt
```

Follow the project’s existing `eckit::testing` and `ecbuild_add_test` conventions.

If ProductTimeSpec tests already exist in the local worktree, extend those instead of creating a duplicate target.

---

## 8.2 Critical anchor tests

Test every non-empty source combination from the truth table.

At minimum verify:

### `date/time` only

Expected:

```text
label = date/time
initialConditions = date/time
reference = date/time
```

### `date/time` plus `hdate`

Expected:

```text
label = hdate 00:00:00
initialConditions = date/time
reference = date/time
```

### `date/time` plus `year/month`

Expected:

```text
label = date/time
initialConditions = date/time
reference = first day of year/month at 00:00:00
```

### all three

Expected:

```text
label = hdate 00:00:00
initialConditions = date/time
reference = year/month anchor
```

### `hdate` only

Expected all three to equal `hdate 00:00:00`.

### `year/month` only

Expected all three to equal the first day of the specified month at midnight.

### `hdate` plus `year/month`

Expected:

```text
label = hdate
initialConditions = hdate
reference = year/month anchor
```

### no sources

Must throw a nested `Mars2GribModelException`.

Also test ordering rejection, for example a hindcast label later than initial conditions.

---

## 8.3 Outer support equality tests

Create products where domain construction and windows agree:

```text
domainStart + outerRange == domainEnd
```

Construction must succeed.

Create a direct helper-level test, or deliberately mutated component test if accessible, where the canonical outer window range does not reproduce `domainEnd`.

Validation must throw.

Because component structures are currently public, helper-level tests may construct invalid artifacts directly. This is acceptable for testing the final consistency stage.

---

## 8.4 Increment bound tests

### Fixed units

Successful:

```text
range = 6 hours
increment = 1 hour
```

Failure:

```text
range = 1 hour
increment = 2 hours
```

### Mixed fixed units

Successful:

```text
range = 1 hour
increment = 3600 seconds
```

Failure:

```text
range = 3599 seconds
increment = 1 hour
```

### Calendar units

Anchor at a valid first-of-month midnight.

Successful:

```text
range = 1 month
increment = 1 day
```

The comparison must be made through DateTime endpoints, not unit ordering.

### AIFS missing increment

For:

```text
class = "ml"
realWindowCount = 1
incrementType = AifsPureMissingIncrement
```

the stored zero sentinel must not be subjected to the semantic increment-bound check. Existing AIFS sentinel validation must still run.

---

## 8.5 Adjacent hierarchy tests

### Successful fixed hierarchy

```text
outer range = 24 hours
inner range = 6 hours
```

### Invalid fixed hierarchy

```text
outer range = 6 hours
inner range = 12 hours
```

Must throw in final consistency, not in window construction.

### Calendar-aware hierarchy

At a first-of-month midnight, test combinations involving months and days. Expected ordering must be determined from the resulting DateTimes.

Do not assert ordering by unit enum or raw length.

---

## 8.6 Exact divisibility option tests

### Option disabled

For a non-divisible adjacent pair, construction must behave exactly as before, provided ordinary hierarchy holds.

Example:

```text
outer = 10 hours
inner = 6 hours
requireExactWindowDivisibility = false
```

Hierarchy holds, exact divisibility does not. The product must not fail solely because of divisibility.

### Option enabled, divisible

```text
outer = 12 hours
inner = 6 hours
requireExactWindowDivisibility = true
```

Must succeed.

### Option enabled, not divisible

```text
outer = 10 hours
inner = 6 hours
requireExactWindowDivisibility = true
```

Must throw.

### Calendar month sequential addition

Use a first-of-month anchor.

Example:

```text
outer = 3 months
inner = 1 month
```

Must succeed after three sequential additions.

### Mixed calendar range

Choose a valid aligned case and verify using the exact sequential algorithm. The test expectation must be derived from repeated DateTime addition, not seconds conversion.

### Every adjacent pair

Use at least three real windows and make the second pair invalid while the first pair is valid. Validation must still fail, proving that all adjacent pairs are checked.

---

## 8.7 Zero-length from-start tests

### Option disabled

```text
shape = FromStartSingleLoop
step = 0
allowZeroLengthFsWindow = false
```

Shape classification must reject it.

### Option enabled and valid

```text
shape = FromStartSingleLoop
step = 0
allowZeroLengthFsWindow = true
domainStart == domainEnd
outerRange.length == 0
```

Must succeed even if the ordinary range/increment or divisibility checks would otherwise be inappropriate.

### Invalid zero-domain relation

With the exception active, construct or inject:

```text
domainStart != domainEnd
```

Final consistency must throw.

### Invalid zero range

With the exception active, construct or inject:

```text
outerRange.length != 0
```

Final consistency must throw.

### Exact-divisibility option also enabled

Enable both:

```text
allowZeroLengthFsWindow = true
requireExactWindowDivisibility = true
```

The valid zero-length from-start representation must still succeed. This proves divisibility is skipped for the exception.

---

## 8.8 Default increment representation tests

Verify the stored `TimeDuration` exactly, including its unit.

| Innermost range | Expected default increment |
|---|---|
| `3600 seconds` | `{600, Second}` |
| `7200 seconds` | `{3600, Second}` |
| `1 hour` | `{600, Second}` |
| `2 hours` | `{3600, Second}` |
| `1 day` | `{3600, Second}` |
| `1 month` | `{3600, Second}` |

A range shorter than one hour must still be rejected.

Do not change explicit increment tests; explicit whole-hour increments may remain represented in hours.

---

## 8.9 Options tests

Test all input paths that are active in the delivery.

### Default

```cpp
Options{}.requireExactWindowDivisibility == false
```

### Direct Options object

Set the member to `true` and ensure ProductTimeSpecInput receives `true`.

### LocalConfiguration

Valid boolean:

```yaml
requireExactWindowDivisibility: true
```

must parse.

Wrong type must throw:

```yaml
requireExactWindowDivisibility: "true"
```

unless the configuration adapter normally performs strict boolean coercion; follow existing option semantics exactly.

### Initializer list

If active:

```cpp
{"requireExactWindowDivisibility", true}
```

must parse.

A non-boolean value must throw.

### Dictionary traits

Test:

```cpp
has(opts, "requireExactWindowDivisibility")
has<bool>(opts, "requireExactWindowDivisibility")
get_opt<bool>(opts, "requireExactWindowDivisibility")
get_or_throw<bool>(opts, "requireExactWindowDivisibility")
```

Also verify requesting the key as the wrong type produces the established dictionary exception.

### Diagnostic JSON

`ProductTimeSpecInput::to_json()` must contain:

```json
"requireExactWindowDivisibility":true
```

or `false`, matching the normalized input.

---

## 8.10 Time utility overflow tests

Add direct tests for the checked helpers.

At minimum:

- multiplication overflow in `year * 12`;
- addition overflow when applying month displacement;
- negative remainder path without underflow;
- normal positive and negative month shifts;
- first-of-month/midnight alignment rejection;
- large but representable values.

The tests should verify an exception is thrown before undefined signed overflow occurs.

---

## 8.11 JSON escaping tests

Test:

```text
quote
backslash
backspace
form feed
newline
carriage return
tab
0x01
0x1F
ordinary ASCII
```

Every result must be valid JSON and must not contain raw control characters below `0x20`.

---

## 8.12 Header self-containment tests

Where practical, add compile-only tests or a small generated-header test target.

At minimum, locally compile one translation unit per public header with that header included first.

---

# 9. File-by-file checklist

## Critical and High

### `backend/deductions/dateTime.h`

- [ ] Replace “direct label datetime” with “direct initial-conditions datetime”.
- [ ] Update return and exception documentation.
- [ ] Do not change parsing.

### `backend/deductions/hindcastDateTime.h`

- [ ] Explicitly document `hdate` as the direct label source.
- [ ] Do not change parsing.

### `backend/deductions/typeOfTimeIncrement.h`

- [ ] Add a prominent unresolved-logic warning.
- [ ] Keep deliberate unconditional failure.

### `api/Options.h`

- [ ] Add default `requireExactWindowDivisibility = false`.
- [ ] Add member with complete documentation.

### `api/Mars2Grib.cc`

- [ ] Parse `requireExactWindowDivisibility` from LocalConfiguration.

### `api/readOptionsFromLocalConfiguration.cc`

- [ ] Mirror the new boolean key.
- [ ] Keep only if this standalone parser remains part of the source tree.

### `api/readOptionsFromInitializerList.cc`

- [ ] Add the new boolean key.
- [ ] Verify whether this parser is actually integrated before changing public API.
- [ ] Resolve stale nonexistent-option references if the file is compiled.

### `utils/dictionary_traits/dictaccess_options.h`

- [ ] Add the key to `isBoolKey`.
- [ ] Add the key to `getBoolOrThrow`.
- [ ] Test `has`, `get_opt`, and `get_or_throw`.

### `models/product-time-spec/ProductTimeSpecInput.h`

- [ ] Add normalized boolean field.
- [ ] Read it through typed options traits.
- [ ] Emit it in diagnostic JSON.
- [ ] Retain `optional<deductions::TimeDuration>`.

### `models/product-time-spec/ProductTimeSpecConsistency.h`

- [ ] Update documentation table.
- [ ] Add new final-stage helper calls in the specified order.
- [ ] Preserve nested diagnostics.

### `models/product-time-spec/detail/productTimeSpecConsistency_details.h`

- [ ] Add forward outer-domain equality.
- [ ] Add zero-length from-start detection and validation.
- [ ] Add semantic increment-bound validation.
- [ ] Add adjacent range hierarchy validation.
- [ ] Add optional sequential divisibility validation.
- [ ] Skip the three ordinary checks for the enabled zero-length from-start case.
- [ ] Skip semantic increment validation for AIFS missing increment.
- [ ] Retain the global support floor relative to `referenceDateTime`.

### `models/product-time-spec/detail/productTimeSpecWindows_details.h`

- [ ] Change defaulted `1 hour` outputs to `3600 seconds`.
- [ ] Do not change explicit increment canonicalization.
- [ ] Do not move calendar-aware hierarchy checks here.

### `models/product-time-spec/detail/ProductTimeSpecTimeUtils.h`

- [ ] Add signed checked addition.
- [ ] Add signed checked multiplication.
- [ ] Protect calendar-month linearization.
- [ ] Protect negative-remainder year decrement.
- [ ] Keep calendar-aware operations in this model detail header for now.

### Shape documentation files

- [ ] Add major warning for `FakeSingleLoopDoubleLoop`.
- [ ] Keep the helper returning `false`.

### Tests

- [ ] Add critical anchor tests.
- [ ] Add final consistency tests.
- [ ] Add option plumbing tests.
- [ ] Add calendar divisibility tests.
- [ ] Add zero-length from-start tests.
- [ ] Add default increment unit tests.

## Medium

### `detail/ProductTimeSpecJsonUtils.h`

- [ ] Escape all JSON control bytes below `0x20`.

### All ProductTimeSpec headers

- [ ] Add direct standard-library includes.
- [ ] Compile each header independently.

### Documentation

- [ ] Update stage descriptions and tables.
- [ ] Review Doxygen group classification.

---

# 10. Explicitly out of scope

Do not perform any of the following in this delivery:

- do not modify the legacy `timeIncrementInSeconds.h` path;
- do not remove `FakeSingleLoopDoubleLoop`;
- do not invent the `FakeSingleLoopDoubleLoop` classification logic;
- do not invent the `typeOfTimeIncrement` mapping;
- do not add backward-looking product exceptions or a whitelist;
- do not make ProductTimeSpec assignable;
- do not remove `const` members;
- do not convert templated model functions to fixed prototypes;
- do not introduce `Context_t`;
- do not derive domain construction directly from built windows;
- do not remove the instant sentinel window;
- do not change the AIFS single-window predicate;
- do not reduce nested diagnostics;
- do not hide or redesign component types in this delivery;
- do not add a model-local duration type;
- do not change explicit time-increment canonicalization;
- do not introduce a configurable default increment value;
- do not add an arbitrary loop limit to sequential divisibility.

---

# 11. Recommended delivery order

Because the delivery is already late, apply the work in this order.

## Phase 1 — Critical correctness

1. Correct the `dateTime` documentation.
2. Implement outer-window/domain forward equality.
3. Implement zero-length from-start detection and explicit validation.
4. Implement calendar-aware increment bounds.
5. Implement calendar-aware adjacent range hierarchy.
6. Add focused tests for these checks.
7. Build and run the ProductTimeSpec tests.

Do not begin cleanup before these tests pass.

## Phase 2 — Required option and behaviour

1. Add `requireExactWindowDivisibility` to Options.
2. Add parser and traits support.
3. Add it to ProductTimeSpecInput and JSON.
4. Implement sequential calendar-aware divisibility.
5. Add option-path and divisibility tests.
6. Change defaulted increments to seconds and test exact units.
7. Add unresolved-logic warnings.

## Phase 3 — Safety

1. Add checked calendar-month arithmetic.
2. Add overflow tests.
3. Harden JSON escaping.
4. Add JSON tests.

## Phase 4 — Low-risk cleanup

1. Fix direct includes.
2. Compile headers independently.
3. Update consistency documentation tables.
4. Correct Doxygen grouping if a suitable existing group is available.

## Phase 5 — Final verification

Run:

```bash
git diff --check
```

Build the complete target, not only the new test.

Run all `mars2grib` tests.

Search for stale names and omissions:

```bash
grep -RIn "requireExactWindowDivisibility" \
    src/metkit/mars2grib \
    tests/mars2grib
```

The key should appear in:

```text
Options default
Options member
LocalConfiguration parser
initializer-list parser or retained standalone parser
Options traits
ProductTimeSpecInput field
ProductTimeSpecInput builder
ProductTimeSpecInput JSON
final consistency logic
tests
documentation
```

Search for the incorrect anchor description:

```bash
grep -RIn \
    -e 'direct label datetime source' \
    -e 'dateTime.*direct label' \
    src/metkit/mars2grib
```

Inspect every result.

Search for defaulted hour-valued increments in the ProductTimeSpec windows builder:

```bash
grep -RIn \
    -e 'TimeDuration{1, tables::TimeUnit::Hour}' \
    src/metkit/mars2grib/backend/models/product-time-spec
```

No automatically defaulted increment should remain represented as `1 hour`.

---

# 12. Acceptance criteria

The change is complete only when all of the following are true.

## Semantics

- [ ] `date/time` is documented as the direct initial-conditions source.
- [ ] `hdate` is documented as the direct hindcast label source.
- [ ] All anchor combinations produce the agreed mapping.
- [ ] Every non-instant outer range reproduces `domainEnd` from `domainStart`.
- [ ] Every semantic increment is calendar-aware and no larger than its window range.
- [ ] Every adjacent real-window pair is ordered outer-to-inner.
- [ ] Exact divisibility is checked for every adjacent pair when enabled.
- [ ] Exact divisibility defaults to disabled.
- [ ] Sequential calendar addition is used.
- [ ] The zero-length from-start exception uses the existing option.
- [ ] The zero-length exception explicitly requires equal domain endpoints and zero outer range.
- [ ] The zero-length exception skips increment, hierarchy, and divisibility checks only.
- [ ] AIFS missing increments are not treated as real zero increments.
- [ ] Backward-looking support remains rejected for now.

## Representation

- [ ] The new model uses only `deductions::TimeDuration`.
- [ ] Explicit increments retain existing deduction canonicalization.
- [ ] Defaulted increments are stored as `600 s` or `3600 s`.
- [ ] Instant products retain one sentinel window.
- [ ] ProductTimeSpec members remain `const`.
- [ ] Model functions remain templated.

## API

- [ ] The new option exists and defaults to false.
- [ ] The active LocalConfiguration path supports it.
- [ ] The initializer-list path supports it if that path is active.
- [ ] Options dictionary traits support it.
- [ ] ProductTimeSpecInput stores and serializes it.

## Robustness

- [ ] Calendar-month index arithmetic cannot overflow silently.
- [ ] Diagnostic JSON escapes all control characters.
- [ ] ProductTimeSpec headers do not depend on transitive standard includes.
- [ ] Nested diagnostics remain unchanged.

## Documentation

- [ ] `FakeSingleLoopDoubleLoop` is marked non-operational and unresolved.
- [ ] `typeOfTimeIncrement` is marked non-operational and unresolved.
- [ ] No documentation claims the placeholder logic is complete.

## Tests

- [ ] All new critical and high-priority cases are covered.
- [ ] Full `mars2grib` test suite passes.
- [ ] Complete project build succeeds.

---

# 13. Deferred architectural record

The following items should be recorded for later work but must not delay this delivery.

## Public surface

Only `ProductTimeSpec` should ultimately be visible to external consumers. Anchor, domain, windows, shape classifications, and intermediate aggregates should eventually become implementation details or read-only views.

## Context cache

A future templated `Context_t` should cache the built ProductTimeSpec and be threaded through all mars2grib concepts and deductions that need shared resolved state.

This will be intentionally cross-cutting and should be designed as a separate change.

## Backward-looking products

The current invariant:

```text
domainStartDateTime >= referenceDateTime
```

will eventually need an explicit whitelist for known synoptic or backward-looking products. Do not weaken the global invariant without identified products and tests.

## Placeholder policies

The exact identification policy for:

```text
FakeSingleLoopDoubleLoop
```

and the deduction logic for:

```text
typeOfTimeIncrement
```

remain open design tasks. Both must continue to fail safely or remain unreachable until the mapping is established from real data.

## Legacy removal

The old `timeIncrementInSeconds` deduction should be removed in a dedicated cleanup after all users have migrated. It must not be mixed into this delivery.
