# ProductTimeSpec final

> Status: final normative specification.
>
> This document defines the temporal model produced by `ProductTimeSpec`, the
> supported input domain, the classifier-first resolver architecture, the final
> canonical time-range IR, and the separate backend lowering contracts used by
> `mars2grib`.

---

## 1. Introduction

`ProductTimeSpec` is the single canonical representation of the temporal meaning
of one MARS product inside `mars2grib`.

The design is intentionally compiler-like:

- **frontend**: extraction, parsing, normalization, and classification of raw
  MARS / parameter / option / caller inputs;
- **construction artifacts**: resolved `ProductTimeSpecAnchor`,
  `ProductTimeSpecShape`, and `ProductTimeSpecIncrement` objects;
- **canonical IR**: the final immutable `ProductTimeSpec`, containing the
  resolved anchor, the absolute support interval, and a small-buffer ordered
  range of canonical `ProductTimeWindow` objects;
- **backends**: independent lowering for point-in-time concepts and statistical
  concepts.

The final IR deliberately avoids backend-specific names such as `forecastTime`.
Those names belong to backend lowering, because their meaning differs between
point-in-time products and statistical products.

All temporal consumers must obtain their temporal information from a resolved
`ProductTimeSpec`. They must not reinterpret raw MARS keys independently.

The default policy is strict and fail-fast:

- invalid inputs throw immediately;
- contradictory temporal states throw immediately;
- recognized but unsupported language values throw immediately with explicit
  "not implemented" context;
- no stage returns invalid or partially classified semantic states.

---

## 2. Cases Handled By `ProductTimeSpec`

The enumeration below intentionally mixes four categories:

- shape-level cases: 1, 4, 5, 6, 7;
- anchor-level cases: 2, 3;
- increment-level cases: 8, 9, 10, 11;
- rejection/error cases: 12, 13.

A single product may belong simultaneously to one anchor case, one shape case,
and one increment case. The categories overlap by design.

1. **Instant products**

   Products with no real statistical processing. Their temporal support
   collapses to a single point in time, with
   `windowStartDateTime == windowEndDateTime`.

   In the final canonical IR, instant products are represented by one
   zero-length canonical time range:

   - `typeOfStatisticalProcessing == Missing`;
   - `typeOfTimeIncrement == Missing`;
   - `timeRange == 0 seconds`;
   - `timeIncrement == 0 seconds`.

   This is an IR-normalization device. The point-in-time backend ignores this
   fake range.

   By specification, instant products must be represented in the source language
   with `timespan="none"` and no `stattype` blocks. A missing `timespan` with no
   `stattype` blocks is accepted only by an explicit compatibility option.

2. **Hindcast / reforecast products**

   Products where the simulation initial-conditions time may differ from the
   label time.

3. **Reference-anchor products based on `year` / `month`**

   Products whose reference time is built from the first day of a specified
   `year` / `month` pair rather than inherited directly from `date` / `time` or
   `hdate`.

4. **Standard single-loop statistics**

   Statistical products represented by a duration-valued `timespan` and no
   `stattype`.

5. **Old-style multi-loop statistics**

   Statistical products represented by a duration-valued `timespan` plus one or
   more `stattype` blocks.

   In theory `stattype` may contain an arbitrary number of blocks. The current
   supported MARS language whitelist admits at most two outer blocks. This is a
   supported-domain policy restriction, not a storage restriction of the final
   IR.

6. **FakeDoubleLoop single-loop statistics**

   Single-loop products encoded with `timespan="none"`, exactly one `stattype`
   block, and a `(class, stream)` combination belonging to the
   fakeDoubleLoop-required representation list.

   The single `stattype` block is promoted to the single canonical
   `ProductTimeWindow`.

7. **`timespan="fs"` / from-start single-loop statistics**

   Single-loop products whose support starts exactly at `referenceDateTime`.

8. **AIFS-pure single-window statistical products**

   `class="ml"` products with exactly one real statistical window whose
   increment is semantically missing. The source `timeIncrementInSeconds` may be
   absent. It may also be explicitly present only when the redundant-increment
   policy allows the source value to be ignored.

9. **AIFS-postprocessed products**

   `class="ml"` statistical products with an explicit
   `timeIncrementInSeconds` that is semantically used. This excludes the
   single-real-window AIFS-pure candidate handled by the redundant-increment
   policy.

10. **Strict statistical products with explicit increment**

    Non-AIFS products, and AIFS-postprocessed products, where the innermost
    sampling increment is explicitly known.

11. **Strict statistical products with defaulted increment**

    Eligible non-`ml` statistical products where the increment is missing but
    may be injected through explicit option-driven policy. Non-`ml`
    `FromStartSingleLoop` products are explicitly excluded from defaulting.

12. **Recognized but unsupported inputs**

    Inputs that belong to the broader MARS language but are currently outside
    the supported `ProductTimeSpec` domain. Examples include unsupported
    `timespan` values such as `inst`, `instantaneous`, and sub-hourly
    durations, and unsupported `step` values such as positive sub-hourly or
    otherwise non-hour-aligned durations.

13. **Invalid temporal combinations**

    Inputs whose temporal meaning is contradictory, incomplete, or forbidden by
    policy.

---

## 3. Coarse Specification

### 3.1 Inputs Used By `ProductTimeSpec`

`ProductTimeSpec` is resolved from:

- normalized MARS-derived temporal keys;
- parsed `stattype` blocks, each carrying both a time range and a
  `typeOfStatisticalProcessing`;
- one normalized parameter-side increment source;
- option-side policy values;
- one caller-supplied innermost `typeOfStatisticalProcessing`.

The relevant source concepts are:

- MARS: `date`, `time`, `hdate`, `year`, `month`, `step`,
  `timespan`, `stattype`, `class`, `stream`, `type`;
- par/misc: `timeIncrementInSeconds`;
- opt: `allowDefaultTimeIncrementInSeconds`, `defaultTimeIncrementInSeconds`,
  `allowZeroLengthFsWindow`, `allowNonEnumeratedPositiveIntegerTimespanHours`,
  `allowRedundantTimeIncrement`, `allowMissingTimespanForInstantProduct`, and
  the default `typeOfTimeIncrement` policy;
- caller: `innerMostTypeOfStatisticalProcessing`.

The MARS context keys `class`, `type`, and `stream` are mandatory for the
encoder-level MARS context. `ProductTimeSpec` therefore requires them during
input extraction even when a particular temporal branch uses only a subset of
those keys.

### 3.2 Resolver Pipeline

Resolution is conceptually split into the following stages.

1. **Extraction**

   Read dictionaries once and build a normalized typed `ProductTimeSpecInput`.

2. **Classification**

   Resolve three valid classification axes:

   - `TimeAnchorKind`
   - `ProductTimeSpecShapeKind`
   - `TimeIncrementKind`

3. **Cross-classification check**

   Verify relationships that must hold across independently valid
   classifications.

4. **Construction dispatch**

   Select specialized construction callbacks from static dispatch tables indexed
   by the corresponding classification enum.

5. **Construction-time semantic artifacts**

   Construct:

   - `ProductTimeSpecAnchor`
   - `ProductTimeSpecShape`
   - `ProductTimeSpecIncrement`

6. **Canonicalization**

   Build the final immutable `ProductTimeSpec` from the construction-time
   artifacts. This step materializes the fixed/small-buffer range of canonical
   `ProductTimeWindow` objects.

7. **Final consistency check**

   Validate whole-object invariants on the final canonical IR.

### 3.3 Classification Axes

1. **Time anchor classification**

   Determines how `labelDateTime`, `initialConditionsDateTime`, and
   `referenceDateTime` are sourced. It depends only on the normalized input. It
   also rejects invalid anchor cross-key states, including no direct anchor
   source and `time` without `date`.

2. **Shape classification**

   Determines whether the product is instant, standard single-loop, multi-loop,
   fakeDoubleLoop single-loop, or from-start single-loop. It depends on the
   normalized shape input and, for single-loop statistics, on the mandatory
   `(class, stream)` context used by the fakeDoubleLoop representation policy.

3. **Increment classification**

   Determines whether the product has no increment, an explicit increment, an
   eligible defaulted increment, or an allowed semantically missing increment. It may
   depend on the normalized input and the resolved shape classification.

The subsequent classification consistency check validates relationships between
shape, increment, and the innermost statistical-processing semantics. The
anchor classification selects the anchor construction branch; resolved anchor
ordering is validated during anchor construction and repeated by final
whole-object invariants.

### 3.4 Frontend vs IR vs Backend Ownership

The frontend owns:

- parsing;
- source normalization;
- classification;
- representation-policy checks;
- construction of semantic artifacts.

The final `ProductTimeSpec` owns:

- the resolved anchor;
- the absolute outer support interval;
- the canonical ordered time-range sequence;
- the options snapshot that explains the policy used to construct it;
- JSON serialization for diagnostics.

Backends own only lowering:

- point-in-time lowering uses the anchor and `windowEndDateTime`;
- statistics lowering converts the canonical time-range array from AoS layout
  to the SoA layout required by ecCodes.

---

## 4. Data Structures

### 4.0 Structural Constants

```cpp
constexpr std::size_t inlineProductTimeWindows = 3;
```

Rationale:

- instant products contain one canonical zero-length time range;
- standard single-loop products contain one canonical time range;
- from-start products contain one canonical time range;
- fakeDoubleLoop products contain one canonical time range promoted from the
  single `stattype` block;
- old-style multi-loop products currently contain up to two outer `stattype`
  windows plus one innermost `timespan` window.

The inline capacity of three covers the currently supported domain without heap
allocation. The canonical storage must nevertheless be structurally capable of
representing more than three time ranges through dynamic overflow storage, so
that the storage model is not coupled to the current MARS whitelist limit.

The current supported-domain whitelist for `stattype` admits at most two outer
blocks. This is a validation policy, not the storage limit.

### 4.0.1 Default Time Constant

`ProductTimeSpec` uses one internal default time constant whenever a MARS date
source has no explicit time component.

```cpp
static const eckit::Time defaultMarsTime{0, 0, 0};
```

This constant is used for all of the following construction-time cases:

- resolving `labelDateTime` when `date` is present and raw `time` is absent;
- constructing `initialConditionsDateTime` from `hdate`;
- constructing `referenceDateTime` from `year` / `month`.

`htime` is not a MARS keyword consumed by `ProductTimeSpec`. It is not read,
validated, stored in `ProductTimeSpecInput`, or used for consistency checking.
The synthetic time component associated with `hdate` is always
`defaultMarsTime`.

### 4.1 `ProductTimeDuration`

A time duration is represented by a GRIB-compatible unit and a non-negative
length.

```cpp
struct ProductTimeDuration {
    tables::TimeUnit unit;
    long             length;
};
```

Allowed units inside the current supported `ProductTimeSpec` domain are:

- `tables::TimeUnit::Second`
- `tables::TimeUnit::Hour`
- `tables::TimeUnit::Day`
- `tables::TimeUnit::Month`

Additional units may be supported later without changing the canonical IR.

### 4.2 `ProductTimeWindow`

A canonical time range stores the complete GRIB statistical time-range tuple
needed by ecCodes:

```cpp
struct ProductTimeWindow {
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing;
    tables::TypeOfTimeIncrement         typeOfTimeIncrement;

    ProductTimeDuration timeRange;
    ProductTimeDuration timeIncrement;
};
```

The fields correspond to GRIB Section 4 statistical time-range fields:

| Field | Meaning |
|-------|---------|
| `typeOfStatisticalProcessing` | GRIB `typeOfStatisticalProcessing` |
| `typeOfTimeIncrement` | GRIB `typeOfTimeIncrement` |
| `timeRange.unit` | GRIB `indicatorOfUnitForTimeRange` |
| `timeRange.length` | GRIB `lengthOfTimeRange` |
| `timeIncrement.unit` | GRIB `indicatorOfUnitForTimeIncrement` |
| `timeIncrement.length` | GRIB `timeIncrement` |

Every canonical `ProductTimeWindow` owns its own
`typeOfStatisticalProcessing`. There is no single global statistical-processing
value for the final `ProductTimeSpec`.

### 4.3 `ParsedStatTypeBlock`

The `stattype` parser produces parsed blocks that contain both the period-derived
window length and the operation-derived statistical-processing type.

```cpp
struct ParsedStatTypeBlock {
    ProductTimeDuration timeRange;
    tables::TypeOfStatisticalProcessing typeOfStatisticalProcessing;
};

// Small-buffer sequence with no heap allocation in the currently supported
// domain and dynamic overflow for future/arbitrary stattype block counts.
class ParsedStatTypeBlocks;
```

Example:

```text
stattype = moav_damn
```

parses to:

```cpp
stattypeBlocks[0] = {
    .timeRange = {tables::TimeUnit::Month, 1},
    .typeOfStatisticalProcessing = tables::TypeOfStatisticalProcessing::Average
};

stattypeBlocks[1] = {
    .timeRange = {tables::TimeUnit::Day, 1},
    .typeOfStatisticalProcessing = tables::TypeOfStatisticalProcessing::Minimum
};
```

### 4.4 `TimespanKind`

```cpp
enum class TimespanKind {
    Missing,
    Duration,
    None,
    FromStart
};
```

Meaning:

- `Missing`: the MARS keyword is absent;
- `Duration`: the keyword carries a supported duration value;
- `None`: the keyword is the literal string `"none"`;
- `FromStart`: the keyword is one of `"fs"`, `"from-start"`, `"fromstart"`.

### 4.5 `TimeAnchorKind`

```cpp
enum class TimeAnchorKind : std::size_t {
    LabelOnly,
    Hindcast,
    ForecastAnchor,
    HindcastForecastAnchor,
    Count
};
```

The kind is determined only by the presence of the two special direct anchor
sources:

| Direct `hdate` | Direct `year` / `month` | `TimeAnchorKind` |
|----------------|-------------------------|------------------|
| absent | absent | `LabelOnly` |
| present | absent | `Hindcast` |
| absent | present | `ForecastAnchor` |
| present | present | `HindcastForecastAnchor` |

The presence or absence of `date` affects datetime inheritance, but it does not
introduce another `TimeAnchorKind`.

### 4.6 `ProductTimeSpecShapeKind`

```cpp
enum class ProductTimeSpecShapeKind : std::size_t {
    Instant,
    StandardSingleLoop,
    MultiLoop,
    FakeDoubleLoopSingleLoop,
    FromStartSingleLoop,
    Count
};
```

### 4.7 `TimeIncrementKind`

```cpp
enum class TimeIncrementKind : std::size_t {
    NoIncrement,
    ExplicitIncrement,
    DefaultedIncrement,
    AifsPureMissingIncrement,
    Count
};
```

Meaning:

- `NoIncrement`: instant product;
- `ExplicitIncrement`: a strictly positive increment is present and semantically
  used;
- `DefaultedIncrement`: an eligible non-`ml`, non-from-start statistical product
  received a defaulted increment through policy;
- `AifsPureMissingIncrement`: a `class="ml"` single-window statistical product
  is treated as semantically missing its increment. The source increment may be
  absent, or it may be explicitly present and ignored only when the redundant
  increment policy allows it.

### 4.8 `ProductTimeSpecKind`

The final canonical object keeps the resolved shape kind for diagnostics and
simple backend branching.

```cpp
using ProductTimeSpecKind = ProductTimeSpecShapeKind;
```

An implementation may keep a separate enum if it wants to decouple final IR
naming from frontend shape naming. The values must remain semantically
identical to `ProductTimeSpecShapeKind`.

### 4.9 `ProductTimeSpecOptions`

The final object stores the policy snapshot used during extraction,
classification, and canonicalization.

```cpp
struct ProductTimeSpecOptions {
    bool allowDefaultTimeIncrementInSeconds;
    std::optional<long> defaultTimeIncrementInSeconds;

    bool allowZeroLengthFsWindow;
    bool allowNonEnumeratedPositiveIntegerTimespanHours;
    bool allowRedundantTimeIncrement;
    bool allowMissingTimespanForInstantProduct;

    tables::TypeOfTimeIncrement defaultTypeOfTimeIncrement;
};
```

The boolean fields are source options. `defaultTimeIncrementInSeconds` is the
materialized default sampling increment used only when
`allowDefaultTimeIncrementInSeconds == true`, no explicit source increment is
available, and the product is eligible for defaulting. Non-`ml`
`FromStartSingleLoop` products are never eligible. When the defaulting option is
enabled, `defaultTimeIncrementInSeconds` must be present and strictly positive. When the defaulting option is disabled,
the value is ignored and should normally be absent.

`defaultTypeOfTimeIncrement` is the materialized policy value used for real
statistical time ranges when `typeOfTimeIncrement` is not source-derived. For
`ExplicitIncrement` and `DefaultedIncrement`, it must not be `Missing`. The final
canonical window always stores the resolved value explicitly.

### 4.10 `ProductTimeSpecInput`

`ProductTimeSpecInput` is a normalized typed source snapshot. It exists to avoid
repeated dictionary access and to provide stable context for exception payloads.

It is not the final semantic object.

```cpp
struct ProductTimeSpecInput {
    std::optional<eckit::Date> marsDate;
    std::optional<eckit::Time> marsTime;

    std::optional<eckit::Date> marsHdate;

    std::optional<long> marsYear;
    std::optional<long> marsMonth;

    std::optional<long> stepInSeconds;

    TimespanKind        timespanKind;
    std::optional<long> timespanInSeconds;

    ParsedStatTypeBlocks stattypeBlocks;

    std::string marsClass;
    std::string marsStream;
    std::string marsType;

    std::optional<long> timeIncrementInSeconds;

    tables::TypeOfStatisticalProcessing innerMostTypeOfStatisticalProcessing;

    ProductTimeSpecOptions options;
};
```

Rules:

- the fields are normalized and typed;
- `marsClass`, `marsStream`, and `marsType` are mandatory normalized MARS
  context fields;
- `marsYear` and `marsMonth` are either both present or both absent;
- `stattypeBlocks` is stored in parsed MARS textual order;
- the structure contains no classification results;
- the structure contains no final semantic fields such as
  `windowStartDateTime`, `windowEndDateTime`, or canonical windows;
- recognized but unsupported inputs are rejected before the structure is
  returned.

### 4.11 `ProductTimeSpecClassification`

```cpp
struct ProductTimeSpecClassification {
    TimeAnchorKind             anchorType;
    ProductTimeSpecShapeKind   shapeType;
    TimeIncrementKind          incrementType;
};
```

This structure contains only valid classifications. Invalid states must throw
before a `ProductTimeSpecClassification` is produced.

`ProductTimeSpecClassification` is an internal resolver artifact. It must not be
part of the public deduction API and must not be consumed by downstream temporal
consumers.

### 4.12 `ProductTimeSpecAnchor`

```cpp
struct ProductTimeSpecAnchor {
    eckit::DateTime labelDateTime;
    eckit::DateTime initialConditionsDateTime;
    eckit::DateTime referenceDateTime;

    TimeAnchorKind anchorType;
};
```

### 4.13 `ProductTimeSpecShape`

`ProductTimeSpecShape` is a construction-time artifact. It is not the final
public representation.

```cpp
struct ProductTimeSpecShape {
    eckit::DateTime windowStartDateTime;
    eckit::DateTime windowEndDateTime;

    // Present for StandardSingleLoop, MultiLoop, and FromStartSingleLoop.
    // Absent for Instant and FakeDoubleLoopSingleLoop.
    std::optional<ProductTimeDuration> innerTimeRange;

    // Parsed outer windows from stattype, in outermost -> innermost order.
    ParsedStatTypeBlocks stattypeBlocks;

    bool zeroLengthFromStartWindowByDesign;

    ProductTimeSpecShapeKind shapeType;
};
```

Shape construction determines absolute support placement and structural window
sources. It does not construct final `ProductTimeWindow` objects.

`zeroLengthFromStartWindowByDesign` is construction-time evidence only. It is
not stored in the final `ProductTimeSpec`; the final state is derived from
`kind`, the canonical range, the support interval, the reference time, and the
option snapshot.

### 4.14 `ProductTimeSpecIncrement`

`ProductTimeSpecIncrement` is a construction-time artifact. It carries the
resolved increment policy used to materialize canonical windows.

```cpp
struct ProductTimeSpecIncrement {
    ProductTimeDuration timeIncrement;
    tables::TypeOfTimeIncrement typeOfTimeIncrement;

    TimeIncrementKind incrementType;
};
```

Rules:

- `ExplicitIncrement` and `DefaultedIncrement` materialize a strictly positive
  `timeIncrement` and a non-missing `typeOfTimeIncrement`; the resolved
  `typeOfTimeIncrement` must not be `Missing`;
- `NoIncrement` materializes `timeIncrement == 0 seconds` and
  `typeOfTimeIncrement == Missing`;
- `AifsPureMissingIncrement` materializes the missing-increment sentinel:
  `timeIncrement == 0 seconds` and `typeOfTimeIncrement == Missing`.

### 4.15 `ProductTimeWindows`

The final canonical time ranges are stored in a small-buffer immutable
container.

The normal inline capacity is `inlineProductTimeWindows == 3`. This covers the
currently supported domain without heap allocation. Values exceeding the inline
capacity are stored in dynamic overflow storage.

The public API must provide:

```cpp
class ProductTimeWindows {
public:
    using value_type = ProductTimeWindow;

    std::size_t size() const noexcept;
    std::size_t numberOfTimeRanges() const noexcept;

    const ProductTimeWindow& operator[](std::size_t i) const noexcept;
    const ProductTimeWindow& at(std::size_t i) const;

    // The container must support read-only range iteration.
    // The concrete const-iterator type is implementation-defined.
};
```

Implementation requirements:

- no heap allocation in the normal supported domain;
- read-only range iteration over all windows, including dynamic overflow
  windows;
- `operator[]` may use debug assertions only;
- `at()` may throw for checked access;
- the exact iterator type is not part of the normative semantic interface;
- the container is exposed immutably after `ProductTimeSpec` construction.

The container may be implemented locally or by using an available small-vector
implementation. A plain `std::vector` must not be used as the only storage in the
hot-path object.

### 4.16 Final `ProductTimeSpec`

The final canonical IR is:

```cpp
class ProductTimeSpec {
public:
    const eckit::DateTime& labelDateTime() const noexcept;
    const eckit::DateTime& initialConditionsDateTime() const noexcept;
    const eckit::DateTime& referenceDateTime() const noexcept;

    TimeAnchorKind anchorType() const noexcept;
    const ProductTimeSpecAnchor& anchor() const noexcept;

    const eckit::DateTime& windowStartDateTime() const noexcept;
    const eckit::DateTime& windowEndDateTime() const noexcept;

    std::size_t numberOfTimeRanges() const noexcept;
    std::size_t size() const noexcept;

    const ProductTimeWindow& operator[](std::size_t i) const noexcept;
    const ProductTimeWindow& at(std::size_t i) const;

    const ProductTimeWindows& timeRanges() const noexcept;

    const ProductTimeSpecOptions& options() const noexcept;
    ProductTimeSpecKind kind() const noexcept;
    TimeIncrementKind incrementKind() const noexcept;

    std::string to_json() const;

    // Direct read-only range iteration may be provided as a convenience.
    // Its concrete iterator type is implementation-defined.
private:
    ProductTimeSpecAnchor anchor_;

    eckit::DateTime windowStartDateTime_;
    eckit::DateTime windowEndDateTime_;

    ProductTimeWindows windows_;

    ProductTimeSpecOptions options_;
    ProductTimeSpecKind kind_;
    TimeIncrementKind incrementKind_;
};
```

`size()` and `numberOfTimeRanges()` return the same value. `size()` exists for
range/container ergonomics. `numberOfTimeRanges()` exists to mirror the GRIB /
ecCodes statistical concept and to make statistics-lowering call sites clearer.

The final object must expose read-only iteration either directly or through
`timeRanges()`. The exact iterator implementation is not normative.

For instant products, `numberOfTimeRanges() == 1` because the final IR contains
one canonical zero-length placeholder. This must not be confused with an ecCodes
instruction to encode a statistical time range for point-in-time products.

### 4.17 `ProductTimeSpecStage`

```cpp
enum class ProductTimeSpecStage {
    InputExtraction,
    TimeAnchorClassification,
    ShapeClassification,
    TimeIncrementClassification,
    ClassificationConsistencyCheck,
    TimeAnchorConstruction,
    ShapeConstruction,
    TimeIncrementConstruction,
    CanonicalWindowConstruction,
    ProductTimeSpecConstruction,
    FinalConsistencyCheck
};
```

`ProductTimeSpecConstruction` precedes `FinalConsistencyCheck` because the final
whole-object invariants are evaluated on a complete immutable candidate object.

### 4.18 `Mars2GribProductTimeSpecException`

Internal `ProductTimeSpec` functions throw
`Mars2GribProductTimeSpecException` once a `ProductTimeSpecInput` snapshot is
available.

This exception carries:

- a stage tag;
- a human-readable reason;
- the normalized `ProductTimeSpecInput` serialized as JSON;
- optionally the resolved classification values;
- optionally construction-time artifacts already produced;
- optionally the final `ProductTimeSpec` serialized as JSON if construction has
  reached the canonical IR stage.

Failures that occur before a fully-formed `ProductTimeSpecInput` snapshot exists
are reported through `Mars2GribGenericException` instead.

---

## 5. Detailed Semantics

### 5.1 Canonical Role Of `ProductTimeSpec`

`ProductTimeSpec` is the only temporal IR consumed by temporal backends.

The construction-time artifacts `ProductTimeSpecAnchor`, `ProductTimeSpecShape`,
and `ProductTimeSpecIncrement` are not public backend inputs. They exist to keep
the frontend classifier and branch-specific construction logic clear.

### 5.2 Temporal Fields

The temporal model consists of five central datetimes:

- `labelDateTime`: the time carried by the request label;
- `initialConditionsDateTime`: the simulation initial-conditions time;
- `referenceDateTime`: the time from which step arithmetic is performed;
- `windowStartDateTime`: the start of the product outer support;
- `windowEndDateTime`: the end of the product outer support.

`windowEndDateTime` is always derived from `referenceDateTime` and the resolved
step in seconds:

```text
resolvedStepInSeconds = input.stepInSeconds.value_or(0)
windowEndDateTime = referenceDateTime + resolvedStepInSeconds
```

The `value_or(0)` fallback is valid only after classification has proved that a
missing `step` is allowed, which happens only for `type="an"`.

For statistical products, `windowStartDateTime` is derived from the outermost
canonical time range:

```text
windowStartDateTime = windowEndDateTime - windows[0].timeRange
```

where subtraction is unit-aware:

- `Month` ranges are subtracted on the calendar;
- `Day` ranges are subtracted on the calendar;
- `Hour` ranges are subtracted numerically as whole-hour elapsed durations;
- `Second` ranges are subtracted numerically as elapsed durations.

For every statistical product supported by this version:

```text
referenceDateTime <= windowStartDateTime <= windowEndDateTime
```

Consequently the statistics-backend `forecastTime` is non-negative. A
statistical support interval beginning before `referenceDateTime` is rejected.

For instant products:

```text
windowStartDateTime == windowEndDateTime
```

For `FromStartSingleLoop`, a second candidate is derived independently:

```text
windowStartFromReference = referenceDateTime
```

The two candidates must be equal:

```text
windowStartDateTime == windowEndDateTime - windows[0].timeRange
windowStartDateTime == referenceDateTime
```

A mismatch is a construction or final consistency error. This rule also applies
to the zero-length from-start case.

### 5.3 Duration Canonicalization

`ProductTimeDuration` distinguishes calendar durations from elapsed durations.
This distinction is semantically relevant for date arithmetic and calendar
alignment.

Canonicalization rules:

1. `Month` is used only for calendar-month ranges derived from `stattype` period
   `mo`.
2. `Day` is used for calendar-day ranges derived from `stattype` period `da`.
3. `Hour` is used for elapsed durations that are positive, whole-hour aligned,
   and not explicitly calendar-derived.
4. `Second` is used for zero durations and for elapsed durations that cannot be
   represented as a whole number of hours in the supported domain.
5. Positive `step` and `timespan` values in the current supported domain must be
   whole-hour aligned; therefore their elapsed-duration representation normally
   uses `Hour`.
6. `timeIncrementInSeconds` and `defaultTimeIncrementInSeconds` are normalized to
   the coarsest exactly representable elapsed unit that does not change calendar
   semantics. For example, `3600` seconds becomes `{Hour, 1}`. `86400` seconds
   may become `{Hour, 24}` unless the value is explicitly a calendar-day range
   derived from `stattype`.

Consequences:

- `24h` and `1 day` are not automatically identical in the IR.
- `24h` is an elapsed duration and is subtracted numerically.
- `1 day` from `stattype` is a calendar-aligned day range and is subject to the
  day-alignment rule.

### 5.4 Canonical Time-Range Semantics

The final `ProductTimeSpec` always contains at least one canonical time range.

```text
numberOfTimeRanges() >= 1
```

Ordering is always:

```text
outermost -> innermost
```

For instant products, the single range is fake and zero-length:

```text
kind() == Instant
numberOfTimeRanges() == 1
windows[0].typeOfStatisticalProcessing == Missing
windows[0].typeOfTimeIncrement == Missing
windows[0].timeRange == 0 seconds
windows[0].timeIncrement == 0 seconds
```

For real statistical products, each window corresponds to one real GRIB
statistical time range.

General real-statistical-window rules:

```text
timeRange.length > 0, except the explicitly allowed zero-length from-start branch
typeOfStatisticalProcessing != Missing
```

`typeOfTimeIncrement` and `timeIncrement` are materialized per window.

### 5.5 Real Statistical Window Count

Classification and cross-classification need a real-window count before the
final `ProductTimeSpec` exists. They must use the frontend helper concept:

```cpp
std::size_t realStatisticalWindowCount(
    ProductTimeSpecShapeKind shapeType,
    std::size_t stattypeBlockCount);
```

Its semantics are:

| `shapeType` | Result |
|-------------|--------|
| `Instant` | `0` |
| `StandardSingleLoop` | `1` |
| `FakeDoubleLoopSingleLoop` | `1` |
| `FromStartSingleLoop` | `1`, including the explicitly allowed zero-length case |
| `MultiLoop` | `stattypeBlockCount + 1` |

For `MultiLoop`, the additional one is the innermost duration-valued `timespan`
window.

A final-IR overload may also be provided:

```cpp
std::size_t realStatisticalWindowCount(const ProductTimeSpec& spec) {
    return spec.kind() == ProductTimeSpecKind::Instant
               ? 0
               : spec.numberOfTimeRanges();
}
```

The final overload deliberately derives the count from the canonical range
cardinality rather than returning a shape-dependent constant. Shape-specific
cardinality is enforced independently by the final invariants in Section 5.16.
This prevents a malformed single-loop object containing more than one canonical
range from being silently reported as having one real statistical window.

The fake zero-length instant range is never counted as a real statistical
window. The zero-length from-start range is counted as real because it belongs
to an explicitly statistical branch.

`AifsPureMissingIncrement` classification and related cross-classification rules
must use the frontend helper, not a helper requiring the final canonical object.
The final-IR overload is a convenience and an internal consistency check.

### 5.6 Canonical Window Construction

Canonical window construction combines:

- parsed `stattype` blocks;
- the deduced innermost range;
- the deduced innermost `typeOfStatisticalProcessing`;
- the resolved increment policy.

Conceptually:

```text
stattype -> outer canonical windows
timespan / step + inner type -> innermost canonical window
```

More explicitly:

| Product kind | Canonical windows |
|-------------|-------------------|
| `Instant` | one fake zero-length missing window |
| `StandardSingleLoop` | one innermost window from `timespan` + innermost type |
| `FromStartSingleLoop` | one innermost window from resolved `step` + innermost type |
| `FakeDoubleLoopSingleLoop` | one window promoted from the single `stattype` block |
| `MultiLoop` | parsed `stattype` windows first, then innermost `timespan` window |

For `MultiLoop`, if:

```text
stattype = moav_damn
timespan = 6h
innerMostTypeOfStatisticalProcessing = Accumulation
```

then the canonical windows are:

```text
windows[0] = 1 month, Average
windows[1] = 1 day,   Minimum
windows[2] = 6 hours, Accumulation
```

The per-window `timeIncrement` is materialized as follows:

1. For the innermost real statistical window, use the resolved
   `ProductTimeSpecIncrement`.
2. For an outer window, use the `timeRange` of the immediately inner canonical
   window as the increment between successive fields used by that outer
   statistical process.
3. For `FakeDoubleLoopSingleLoop`, the single promoted `stattype` window is the
   only canonical window, so it uses the resolved `ProductTimeSpecIncrement`.
4. For instant products, use the missing sentinel: `typeOfTimeIncrement ==
   Missing` and `timeIncrement == 0 seconds`.
5. For `AifsPureMissingIncrement`, use the missing-increment sentinel:
   `typeOfTimeIncrement == Missing` and `timeIncrement == 0 seconds`.

For all other real statistical windows, `typeOfTimeIncrement` is the resolved
non-missing policy value, normally `options.defaultTypeOfTimeIncrement`.

After all canonical windows have been materialized, every real window is checked
against the increment-within-window rule in Section 5.12.6.

### 5.7 `timespan` Semantics

The raw MARS `timespan` key has four normalized supported states.

#### 5.7.1 Missing

The key is absent. By specification this is not the normative instant
representation; instant products must use `timespan="none"`. Missing
`timespan` with no `stattype` blocks is a compatibility instant shape and is
accepted only when `allowMissingTimespanForInstantProduct == true`.

#### 5.7.2 Duration

The key carries a supported duration.

String-valued `timespan` is accepted only when it belongs to the explicit
language-defined set and is supported by `ProductTimeSpec`.

Integer-valued `timespan` is interpreted as hours.

By default, integer-valued `timespan` must map to one of the supported
language-defined values. The option
`allowNonEnumeratedPositiveIntegerTimespanHours == true` bypasses that
restriction and allows any integer hour value `>= 1`.

#### 5.7.3 `"none"`

The key is the literal string `"none"`.

This state has two supported meanings:

- with no `stattype` blocks, it is an instant representation;
- with exactly one `stattype` block and an allowed `(class, stream)` pair, it is
  a fakeDoubleLoop single-loop statistical representation.

All other `timespanKind == None` combinations are hard errors.

#### 5.7.4 `"fs"` / from-start aliases

The key is one of:

- `"fs"`
- `"from-start"`
- `"fromstart"`

This state defines the from-start single-loop representation.

#### 5.7.5 Recognized But Unsupported `timespan` Values

The following values are recognized but rejected during extraction:

- `"inst"`
- `"instantaneous"`
- sub-hourly supported language values such as `10m`, `15m`, `20m`, `30m`

These must throw `Mars2GribGenericException` with explicit "not implemented"
context, because they are detected before a fully-formed `ProductTimeSpecInput`
snapshot exists.

### 5.8 `stattype` Semantics

`stattype` is parsed as a sequence of blocks:

```text
stattype  := block ('_' block)*
block     := period operation
period    := 'mo' | 'da'
operation := 'av' | 'ac' | 'mn' | 'mx' | 'sd'
```

Examples:

```text
moav       -> monthly average
damn       -> daily minimum
moav_damn  -> monthly average of daily minima
```

The full textual value must remain consistent with the MARS language definition
in `share/metkit/language.yaml`. The grammar describes how to parse a candidate
value. It does not by itself define the supported domain.

Validation is two-stage.

1. **Grammar parsing**

   If the text is not a valid `stattype` grammar instance, a lower-level parsing
   error is thrown.

2. **Language membership validation**

   If the text is grammatically valid but not listed in the language definition,
   a different lower-level validation error is thrown.

At the `ProductTimeSpec` extraction layer, both failures occur before a complete
`ProductTimeSpecInput` snapshot exists and are reported through
`Mars2GribGenericException`.

Current supported-domain ordering rules:

- at most one `mo` block;
- at most one `da` block;
- if both are present, `mo` must precede `da`.

This gives at most two outer blocks in the currently supported domain. The final
canonical storage is not structurally limited to two outer blocks.

Each block maps to both a range and a type:

| Token part | Mapping |
|------------|---------|
| `mo` | `timeRange = {Month, 1}` |
| `da` | `timeRange = {Day, 1}` |
| `av` | `typeOfStatisticalProcessing = Average` |
| `ac` | `typeOfStatisticalProcessing = Accumulation` |
| `mn` | `typeOfStatisticalProcessing = Minimum` |
| `mx` | `typeOfStatisticalProcessing = Maximum` |
| `sd` | `typeOfStatisticalProcessing = StandardDeviation` |

The exact enum names must match the table namespace used by the implementation.

### 5.9 Time-Anchor Semantics

The anchor stage resolves three ordered datetimes:

- `labelDateTime`
- `initialConditionsDateTime`
- `referenceDateTime`

The semantic ordering is:

```text
labelDateTime <= initialConditionsDateTime <= referenceDateTime
```

Define the three possible direct-source values:

```text
L = DateTime(date, time.value_or(defaultMarsTime))
H = DateTime(hdate, defaultMarsTime)
R = DateTime(Date(year, month, 1), defaultMarsTime)
```

where:

- `L` exists only when `date` is present;
- `H` exists only when `hdate` is present;
- `R` exists only when both `year` and `month` are present.

The three datetimes are resolved by hierarchical inheritance:

```text
labelDateTime -> initialConditionsDateTime -> referenceDateTime
```

The complete direct-source and classification matrix is:

| `date` | `hdate` | `year` + `month` | `labelDateTime` | `initialConditionsDateTime` | `referenceDateTime` | `TimeAnchorKind` |
|--------|---------|------------------|-----------------|---------------------------------|---------------------|------------------|
| present | absent | absent | `L` | `L` | `L` | `LabelOnly` |
| present | present | absent | `L` | `H` | `H` | `Hindcast` |
| present | absent | present | `L` | `L` | `R` | `ForecastAnchor` |
| present | present | present | `L` | `H` | `R` | `HindcastForecastAnchor` |
| absent | present | absent | `H` | `H` | `H` | `Hindcast` |
| absent | present | present | `H` | `H` | `R` | `HindcastForecastAnchor` |
| absent | absent | present | `R` | `R` | `R` | `ForecastAnchor` |
| absent | absent | absent | hard error | hard error | hard error | none |

The `TimeAnchorKind` is determined only by direct `hdate` and direct
`year` / `month` presence. A missing `date` does not create another kind; the
label time inherits according to the table.

After the table is applied, the ordering invariant is checked. For example,
`hdate` later than the `year` / `month` reference anchor is an
anchor-construction error, not a new inheritance mode.

Resolution and validation rules:

1. A datetime with a direct source takes its direct-source value.
2. A datetime without a direct source inherits from the nearest resolved
   datetime in the hierarchy.
3. If `initialConditionsDateTime` has no direct source while both
   `labelDateTime` and `referenceDateTime` have direct sources,
   `initialConditionsDateTime` inherits from `labelDateTime`.
4. At least one of `date`, `hdate`, or the complete `year` / `month` pair must
   be present.
5. `time` without `date` is a hard `TimeAnchorClassification` error.
6. `year` and `month` must be both present or both absent.
7. `month` must be in `[1, 12]`, and `Date(year, month, 1)` must be valid.
8. After inheritance, the resolved datetimes must satisfy:

```text
labelDateTime <= initialConditionsDateTime <= referenceDateTime
```

A missing direct anchor source set is a hard error during
`TimeAnchorClassification`. An ordering violation after inheritance is a hard
error during `TimeAnchorConstruction`.

### 5.10 Shape Semantics

The structural shape-classification table is:

| `timespanKind` | `stattypeBlocks.size()` | Additional condition | Shape |
|----------------|-------------------------|----------------------|-------|
| `Missing` | `0` | `allowMissingTimespanForInstantProduct == true` | `Instant` |
| `Missing` | `0` | `allowMissingTimespanForInstantProduct == false` | hard error |
| `Missing` | `>0` | none | hard error |
| `None` | `0` | none | `Instant` |
| `None` | `1` | fakeDoubleLoop required | `FakeDoubleLoopSingleLoop` |
| `None` | `1` | fakeDoubleLoop not required | hard error |
| `None` | `>1` | none | hard error |
| `Duration` | `0` | fakeDoubleLoop not required | `StandardSingleLoop` |
| `Duration` | `0` | fakeDoubleLoop required | hard error |
| `Duration` | `>0` | language-whitelisted value | `MultiLoop` |
| `FromStart` | `0` | zero-length policy satisfied if step resolves to 0 | `FromStartSingleLoop` |
| `FromStart` | `>0` | none | hard error |

By specification, instant products must be represented with `timespan="none"`.
The `Missing` / `0` instant row is a compatibility representation and is valid
only when `allowMissingTimespanForInstantProduct == true`. Otherwise it is a
shape-classification error.

For `FromStartSingleLoop`, a resolved step of zero is accepted only when
`allowZeroLengthFsWindow == true`. Otherwise it is a shape-classification error.

### 5.11 Shape Construction

Shape construction produces `ProductTimeSpecShape`.

#### 5.11.1 Instant

```text
windowStartDateTime == windowEndDateTime
innerTimeRange == nullopt
stattypeBlocks.empty()
```

#### 5.11.2 StandardSingleLoop

```text
innerTimeRange = duration from timespan
stattypeBlocks.empty()
windowEndDateTime = referenceDateTime + resolvedStep
windowStartDateTime = windowEndDateTime - innerTimeRange
```

#### 5.11.3 MultiLoop

```text
stattypeBlocks = parsed outer blocks from stattype
innerTimeRange = duration from timespan
windowEndDateTime = referenceDateTime + resolvedStep
windowStartDateTime = windowEndDateTime - outermost(stattypeBlocks).timeRange
```

Outer blocks are already ordered outermost to innermost.

#### 5.11.4 FakeDoubleLoopSingleLoop

```text
stattypeBlocks.size() == 1
innerTimeRange == nullopt
windowEndDateTime = referenceDateTime + resolvedStep
windowStartDateTime = windowEndDateTime - stattypeBlocks[0].timeRange
```

The single `stattype` block is later promoted to the single canonical
`ProductTimeWindow`.

#### 5.11.5 FromStartSingleLoop

```text
innerTimeRange = duration from resolved step
windowStartDateTime == referenceDateTime
windowEndDateTime = referenceDateTime + resolvedStep
```

Construction must also verify:

```text
windowStartDateTime == windowEndDateTime - innerTimeRange
```

For zero-length from-start products:

```text
resolvedStep == 0
innerTimeRange == 0 seconds
zeroLengthFromStartWindowByDesign == true
```

The boolean is a construction-time assertion only and is not copied into the
final canonical object. This is the only real statistical branch where a
zero-length time range is allowed.

### 5.12 Time-Increment Semantics

#### 5.12.1 Classification Table

The frontend obtains the real-window count from Section 5.5 before classifying
the increment.

| Shape / input | Classification |
|---------------|----------------|
| `Instant` and no increment | `NoIncrement` |
| `Instant` and explicit increment with redundant option disabled | hard error |
| `Instant` and explicit increment with redundant option enabled | `NoIncrement`, explicit value ignored |
| `class == "ml"`, exactly one real statistical window, missing increment | `AifsPureMissingIncrement` |
| `class == "ml"`, exactly one real statistical window, explicit positive increment with redundant option disabled | hard error |
| `class == "ml"`, exactly one real statistical window, explicit positive increment with redundant option enabled | `AifsPureMissingIncrement`, explicit value ignored |
| remaining statistical product with explicit positive increment | `ExplicitIncrement` |
| `class != "ml"` and `shape == FromStartSingleLoop`, missing increment | hard error |
| eligible `class != "ml"` and non-from-start statistical product, missing increment, default option enabled with valid positive default increment and non-missing default type | `DefaultedIncrement` |
| otherwise missing increment | hard error |

The rows are evaluated in order. In particular:

- the explicit-increment `class == "ml"` single-real-window rows take precedence
  over the remaining explicit-increment row;
- the non-`ml` from-start missing-increment row takes precedence over the general
  defaulting row.

Therefore an explicit increment on an AIFS-pure single-window candidate is
either rejected or ignored under the redundant policy; it is not classified as
`ExplicitIncrement`.

`AifsPureMissingIncrement` is allowed only for exactly one real statistical
window. It is not allowed for multi-window statistical products. It represents
a semantically missing increment even when an explicit source increment was
present and ignored under the redundant-increment policy.

A non-`ml` `FromStartSingleLoop` must carry an explicit positive
`timeIncrementInSeconds`. It must never receive `DefaultedIncrement`, even when
defaulting is generally enabled. In from-start semantics the statistical window
changes with `step`; defaulting could otherwise produce a time history of the
same field whose effective sampling increment varies during the initial steps.

#### 5.12.2 `NoIncrement`

`NoIncrement` is legal only for instant products.

Final canonical instant window:

```text
typeOfStatisticalProcessing == Missing
typeOfTimeIncrement == Missing
timeRange == 0 seconds
timeIncrement == 0 seconds
```

#### 5.12.3 `ExplicitIncrement`

The explicit positive `timeIncrementInSeconds` is converted to a
`ProductTimeDuration` with a supported unit and positive length.

The conversion follows Section 5.3 duration canonicalization. For example:

```text
3600 seconds  -> 1 hour
86400 seconds -> 24 hours, unless the value is explicitly calendar-derived
```

If a future implementation restricts time-increment units to seconds for a
backend-specific reason, it may lower the canonical elapsed duration to seconds
inside the backend. The canonical IR still keeps the resolved unit explicitly.

#### 5.12.4 `DefaultedIncrement`

The default value is injected through explicit option-driven policy. This is
allowed only for eligible non-`ml` statistical products and is explicitly
forbidden for `FromStartSingleLoop`.

When `allowDefaultTimeIncrementInSeconds == true`,
`options.defaultTimeIncrementInSeconds` must be present and strictly positive.
That value is normalized to a `ProductTimeDuration` and becomes the materialized
innermost increment.

When `allowDefaultTimeIncrementInSeconds == false`, a missing source increment
must not classify as `DefaultedIncrement`.

The final canonical windows store the materialized default increment, not a
marker saying that an increment should be defaulted later.

#### 5.12.5 `AifsPureMissingIncrement`

`AifsPureMissingIncrement` represents a known semantic regime: a `class="ml"`
single-window statistical product whose increment is semantically missing.

The source `timeIncrementInSeconds` may be absent. It may also be explicitly
present only when `options.allowRedundantTimeIncrement == true`; in that case
the explicit source value is treated as redundant and ignored. If the explicit
value is present and the redundant-increment option is disabled, classification
throws a hard error.

The final canonical single real window uses the missing-increment sentinel:

```text
typeOfTimeIncrement == Missing
timeIncrement == 0 seconds
```

This semantic state remains explicit through `ProductTimeSpec::incrementKind()`.

#### 5.12.6 Increment Must Fit Inside Each Statistical Window

For every real canonical statistical window:

```text
timeIncrement <= timeRange
```

The comparison is semantic and unit-aware. It must not compare raw
`(unit, length)` pairs lexicographically.

For canonical window `i`, define its realized start as:

```text
realizedWindowStart[i] =
    windowEndDateTime - windows[i].timeRange
```

using the unit-aware arithmetic of Section 5.2. Compare the two realized
endpoints obtained from that same start:

```text
realizedWindowStart[i] + windows[i].timeIncrement
    <=
realizedWindowStart[i] + windows[i].timeRange
```

Calendar-derived `Day` and `Month` values therefore use the actual calendar
placement of the product. Elapsed `Hour` and `Second` values are compared exactly
as elapsed time.

The only exception is the explicitly allowed zero-length from-start product:

```text
shape == FromStartSingleLoop
resolvedStepInSeconds == 0
```

In that case the real statistical window has `timeRange == 0 seconds` while its
required explicit sampling increment may be strictly positive.

For multi-loop products this rule is checked for every real window:

- the innermost window validates the resolved explicit/defaulted/missing
  increment;
- each outer window validates the immediately inner `timeRange` used as its
  increment.

This catches invalid nesting where an inner statistical period is longer than
the outer statistical window.

### 5.13 FakeDoubleLoop Representation Policy

FakeDoubleLoop is a structural single-loop representation.

It is characterized by exactly this input shape:

```text
timespanKind == None
stattypeBlocks.size() == 1
```

`timespanKind == None` has one additional meaning: with zero `stattype` blocks it
is an instant representation.

The fakeDoubleLoop-required representation policy is evaluated only for the two
candidate single-loop statistical source representations:

```text
Duration + 0 stattype blocks:
    StandardSingleLoop, valid only when
    requiresFakeDoubleLoopRepresentation(class, stream) == false

None + 1 stattype block:
    FakeDoubleLoopSingleLoop, valid only when
    requiresFakeDoubleLoopRepresentation(class, stream) == true
```

The policy is not applied to `Instant`, `MultiLoop`, or
`FromStartSingleLoop`. In particular, a genuine old-style multi-loop product is
not rejected merely because its `(class, stream)` pair appears in the
fakeDoubleLoop-required representation list.

`timespanKind == None` with more than one `stattype` block is a loud hard error
during shape classification. Such an input is not a multi-loop representation
and must not be interpreted as one.

`MultiLoop` is only the old-style representation:

```text
timespanKind == Duration
stattypeBlocks.size() > 0
```

Therefore, fakeDoubleLoop and multi-loop are disjoint source representations.

Define:

```cpp
bool requiresFakeDoubleLoopRepresentation(std::string_view marsClass,
                                          std::string_view marsStream);
```

The current authoritative fakeDoubleLoop-required representation list is:

| `class` | `stream` values |
|---------|-----------------|
| `e6`    | `sttd`, `stte` |
| `od`    | `sfmd`, `shmd` |
| `rd`    | `sfmd`, `shmd` |
| `c3`    | `sfmd`, `shmd` |
| `gh`    | `msmm`, `rfsd` |
| `eh`    | `msmm`, `rfsd` |

For `FakeDoubleLoopSingleLoop`, all of the following must hold:

1. `timespanKind == None`
2. `stattypeBlocks.size() == 1`
3. `requiresFakeDoubleLoopRepresentation(class, stream) == true`
4. `innerMostTypeOfStatisticalProcessing ==
   stattypeBlocks[0].typeOfStatisticalProcessing`

The last rule is a cross-source consistency check. The single `stattype` block
and the caller-supplied innermost processing value describe the same semantic
operation. A mismatch is a hard cross-classification error and must not be
silently ignored.

For `StandardSingleLoop`, all of the following must hold:

1. `timespanKind == Duration`
2. `stattypeBlocks.empty()`
3. `requiresFakeDoubleLoopRepresentation(class, stream) == false`

If a product requiring fakeDoubleLoop is represented as standard single-loop, it
is a shape-classification error. If a product not requiring fakeDoubleLoop is
represented as fakeDoubleLoop, it is also a shape-classification error.

### 5.14 Calendar Alignment Rules

For calendar-aligned outermost windows, strict alignment is required.

If the outermost canonical time range is day-based:

```text
windowEndDateTime must be at hh=00, mm=00, ss=00
```

If the outermost canonical time range is month-based:

```text
windowEndDateTime must be on day=1 at hh=00, mm=00, ss=00
```

Misaligned ends are hard errors.

### 5.15 Backend Naming Semantics

`forecastTime` is not a canonical `ProductTimeSpec` field. It is a backend
encoding key.

Backend mapping:

| Backend | ecCodes key | Source from `ProductTimeSpec` |
|---------|-------------|-------------------------------|
| Point-in-time | `forecastTime` | `windowEndDateTime - referenceDateTime` |
| Statistics | `forecastTime` | `windowStartDateTime - referenceDateTime` |
| Statistics | `endOfOverallTimeInterval` | `windowEndDateTime` |
| Statistics | `numberOfTimeRanges` | `numberOfTimeRanges()` |
| Statistics | repeated time-range fields | `operator[]` / iteration over windows |

The point-in-time backend consumes only:

- `labelDateTime`
- `initialConditionsDateTime`
- `referenceDateTime`
- `windowEndDateTime`

The statistics backend consumes:

- `referenceDateTime`
- `windowStartDateTime`
- `windowEndDateTime`
- the canonical time-window range.

### 5.16 Final `ProductTimeSpec` Invariants

The final immutable `ProductTimeSpec` must satisfy at least the following
invariants.

1. `labelDateTime <= initialConditionsDateTime <= referenceDateTime`
2. `windowStartDateTime <= windowEndDateTime`
3. every statistical product satisfies
   `referenceDateTime <= windowStartDateTime`
4. `numberOfTimeRanges() >= 1`
5. `numberOfTimeRanges() == timeRanges().size()`
6. `Instant` implies `numberOfTimeRanges() == 1`
7. `Instant` implies `windowStartDateTime == windowEndDateTime`
8. `Instant` implies the single canonical window has:
   - `typeOfStatisticalProcessing == Missing`
   - `typeOfTimeIncrement == Missing`
   - `timeRange == 0 seconds`
   - `timeIncrement == 0 seconds`
9. `StandardSingleLoop`, `FakeDoubleLoopSingleLoop`, and
   `FromStartSingleLoop` each imply `numberOfTimeRanges() == 1`
10. `MultiLoop` implies `numberOfTimeRanges() >= 2`
11. every non-instant product satisfies
    `realStatisticalWindowCount(spec) == numberOfTimeRanges()`
12. every non-instant product satisfies:

    ```text
    windowStartDateTime ==
        subtractDuration(windowEndDateTime, timeRanges()[0].timeRange)
    ```

    using the unit-aware arithmetic of Section 5.2
13. every real canonical time range has
    `typeOfStatisticalProcessing != Missing`
14. all real statistical time ranges have `timeRange.length > 0`, except the
    explicitly allowed zero-length from-start branch
15. `FromStartSingleLoop` implies
    `windowStartDateTime == referenceDateTime`
16. zero-length real statistical support is legal only when all of the following
    hold:
    - `kind() == FromStartSingleLoop`
    - `numberOfTimeRanges() == 1`
    - `timeRanges()[0].timeRange == 0 seconds`
    - `windowStartDateTime == windowEndDateTime == referenceDateTime`
    - `options().allowZeroLengthFsWindow == true`
17. `AifsPureMissingIncrement` requires
    `realStatisticalWindowCount(spec) == 1`
18. `AifsPureMissingIncrement` implies that its single real canonical window has
    `typeOfTimeIncrement == Missing` and `timeIncrement == 0 seconds`, regardless
    of whether the source increment was absent or explicitly ignored as redundant
19. `NoIncrement` is legal only for `Instant`
20. `ExplicitIncrement` implies a positive innermost `timeIncrement`
21. `DefaultedIncrement` implies a materialized positive innermost
    `timeIncrement`
22. `DefaultedIncrement` is illegal for `FromStartSingleLoop`; its
    non-`ml` eligibility is guaranteed by classification
23. `ExplicitIncrement` and `DefaultedIncrement` imply a non-`Missing`
    `typeOfTimeIncrement`
24. `NoIncrement` implies all canonical time-increment fields are the missing
    instant sentinel
25. for multi-loop products, windows are ordered outermost to innermost
26. for outer windows, `timeIncrement` equals the `timeRange` of the immediately
    inner window, unless a future explicit policy overrides this rule
27. for the innermost real window, `timeIncrement` is the resolved
    `ProductTimeSpecIncrement`
28. for every real canonical window, `timeIncrement <= timeRange`, except when
    the object is the explicitly allowed zero-length `FromStartSingleLoop`
    described by invariant 16
29. calendar-aligned outer windows satisfy Section 5.14.

Invariant 3 intentionally rejects statistical products whose support starts
before `referenceDateTime`; negative statistical-backend `forecastTime` is not
supported by this version.

Invariant 28 uses semantic duration comparison as specified in Section 5.12.6.

### 5.17 Error Families

The implementation must throw on every invalid temporal state. The exact message
text is an implementation concern, but each failure must belong to one of the
following semantic families.

#### 5.17.1 Input Errors

These failures arise while reading, parsing, or normalizing source values before
a complete `ProductTimeSpecInput` snapshot exists.

1. malformed `date` or malformed `time`;
2. only one of `year` / `month` present;
3. invalid `year` or `month`, including `month` outside `[1, 12]` or failure to
   construct `Date(year, month, 1)`;
4. negative `step`;
5. invalid integer `timespan <= 0`;
6. malformed or language-invalid `stattype`;
7. `timeIncrementInSeconds <= 0`;
8. `allowDefaultTimeIncrementInSeconds == true` with missing or non-positive
   `defaultTimeIncrementInSeconds`;
9. missing mandatory MARS context key `class`, `type`, or `stream`.

Recognized but unsupported values, including `timespan=inst`,
`timespan=instantaneous`, sub-hourly `timespan`, and positive sub-hourly or
otherwise non-hour-aligned `step`, are reported explicitly as not implemented.

#### 5.17.2 Classification And Cross-Classification Errors

These failures arise when normalized inputs do not map to a valid semantic
classification, or when independently valid local classifications form an
illegal classification combination.

1. no direct anchor source present;
2. `marsTime.has_value()` with `!marsDate.has_value()`;
3. missing `step` for `marsType != "an"`;
4. explicit non-zero `step` for `marsType == "an"`;
5. invalid `timespan` / `stattype` shape combination;
6. missing `timespan` with no `stattype` blocks when
   `allowMissingTimespanForInstantProduct == false`;
7. fakeDoubleLoop representation-policy violation;
8. FakeDoubleLoop caller processing does not match the processing parsed from
   the single `stattype` block;
9. `timespanKind == FromStart` with any `stattype` block;
10. zero-length from-start support when the option is disabled;
11. instant product with an explicit increment when redundant increments are not
    allowed;
12. `class == "ml"` single-window statistical product with an explicit increment
    when redundant increments are not allowed;
13. instant product with
    `innerMostTypeOfStatisticalProcessing != Missing`;
14. `StandardSingleLoop`, `MultiLoop`, or `FromStartSingleLoop` with
    `innerMostTypeOfStatisticalProcessing == Missing`;
15. `FromStartSingleLoop` with innermost processing other than `Accumulation`;
16. non-`ml` `FromStartSingleLoop` with missing
    `timeIncrementInSeconds`, regardless of the defaulting options;
17. multi-window `class == "ml"` product without explicit increment;
18. missing increment where no explicit, eligible defaulted, or allowed-missing
    policy applies;
19. `ExplicitIncrement` or `DefaultedIncrement` with
    `defaultTypeOfTimeIncrement == Missing`;
20. illegal combination of individually valid classifications.

#### 5.17.3 Construction Errors

These failures arise when a specialized construction callback cannot construct a
valid construction-time artifact from a valid classification and normalized
input.

Examples include:

- branch-specific mandatory source missing;
- anchor ordering violation after inheritance;
- disallowed time unit;
- invalid branch-local window count;
- inconsistent from-start support;
- failure to materialize a valid `ProductTimeDuration`.

#### 5.17.4 Canonicalization And Internal Consistency Errors

These failures indicate that construction-time artifacts cannot be assembled into
a valid final canonical `ProductTimeSpec`, or that the final IR violates a
whole-object invariant.

Examples include:

- empty canonical window range;
- invalid instant sentinel;
- missing per-window `typeOfStatisticalProcessing`;
- missing per-window `typeOfTimeIncrement` where a real increment is required;
- statistical support beginning before `referenceDateTime`;
- a real window whose `timeIncrement` exceeds its `timeRange`, outside the
  zero-length from-start exception;
- inconsistent `numberOfTimeRanges()`;
- failed AoS construction for the canonical window sequence.

---

## 6. Classifier And Construction Architecture

### 6.1 Overview

The architecture remains classifier-first.

```text
ProductTimeSpecInput
    -> ProductTimeSpecClassification
    -> ProductTimeSpecAnchor
    -> ProductTimeSpecShape
    -> ProductTimeSpecIncrement
    -> ProductTimeSpec canonical IR
```

The final canonicalization step is described in the following paragraphs.

### 6.2 `classify_TimeAnchor_or_throw`

This classifier determines `TimeAnchorKind` from the direct special-anchor
sources:

| Direct `hdate` | Direct `year` / `month` | Result |
|----------------|-------------------------|--------|
| absent | absent | `LabelOnly` |
| present | absent | `Hindcast` |
| absent | present | `ForecastAnchor` |
| present | present | `HindcastForecastAnchor` |

It rejects:

- no direct anchor source among `date`, `hdate`, and complete `year` / `month`;
- `time` without `date`;
- partial or malformed anchor source combinations not already rejected during
  extraction.

It does not perform final ordering checks, because ordering can only be checked
after inheritance during anchor construction.

### 6.3 `classify_ProductTimeSpecShape_or_throw`

This classifier determines `ProductTimeSpecShapeKind` and owns:

- `timespan` / `stattype` structural compatibility;
- fakeDoubleLoop representation policy;
- from-start rejection of any `stattype` block;
- zero-length from-start rejection when disabled.

### 6.4 `classify_TimeIncrement_or_throw`

This classifier determines `TimeIncrementKind` and owns:

- frontend real-window counting through Section 5.5;
- redundant increment handling for products whose increment is semantically
  missing;
- explicit increment acceptance;
- eligible defaulted increment policy;
- explicit rejection of defaulting for non-`ml` `FromStartSingleLoop`;
- AIFS-pure missing-increment policy, including explicit redundant increment
  rejection or ignoring;
- missing increment rejection.

### 6.5 Cross-Classification Consistency Check

The cross-classification stage verifies relationships between valid local
classifications.

Current rules include:

- `NoIncrement` only with `Instant`;
- `AifsPureMissingIncrement` only with exactly one real statistical window,
  counted by the frontend helper in Section 5.5;
- `DefaultedIncrement` not with `class == "ml"`;
- `DefaultedIncrement` not with `FromStartSingleLoop`;
- instant products must have innermost statistical processing `Missing`;
- `StandardSingleLoop`, `MultiLoop`, and `FromStartSingleLoop` must have
  `innerMostTypeOfStatisticalProcessing != Missing`;
- from-start products must have innermost statistical processing compatible
  with the supported from-start semantics, currently `Accumulation`;
- FakeDoubleLoop caller-supplied innermost processing must equal the non-missing
  processing parsed from its single `stattype` block.

### 6.6 Static Dispatch Tables

Construction dispatch should use static tables indexed by classification enums:

```cpp
using AnchorBuilder = ProductTimeSpecAnchor (*)(const ProductTimeSpecInput&);
using ShapeBuilder  = ProductTimeSpecShape  (*)(const ProductTimeSpecInput&,
                                                const ProductTimeSpecAnchor&);
using IncrementBuilder = ProductTimeSpecIncrement (*)(const ProductTimeSpecInput&,
                                                      const ProductTimeSpecShape&);
```

Dispatch tables must be exhaustive over valid enum values and must reject
`Count` or invalid enum values.

### 6.7 Canonicalization Function

The final canonicalization function consumes construction-time artifacts:

```cpp
ProductTimeSpec make_ProductTimeSpec_or_throw(
    const ProductTimeSpecInput& input,
    const ProductTimeSpecClassification& classification,
    ProductTimeSpecAnchor anchor,
    ProductTimeSpecShape shape,
    ProductTimeSpecIncrement increment);
```

It must:

1. construct the canonical `ProductTimeWindows` sequence;
2. materialize per-window `typeOfStatisticalProcessing`;
3. materialize per-window `typeOfTimeIncrement`;
4. materialize per-window `timeRange` and `timeIncrement`;
5. validate the increment-within-window rule for every real canonical window;
6. store the options snapshot;
7. store `kind` and `incrementKind`;
8. construct the complete immutable `ProductTimeSpec` candidate;
9. perform final consistency checks on that complete candidate;
10. return the validated immutable `ProductTimeSpec`.

## 7. Input Extraction And Normalization

### 7.1 Extraction Contract

Extraction reads dictionaries once and returns `ProductTimeSpecInput` only if all
source-level normalized values are valid and supported.

Unsupported-but-recognized values are rejected here if they can be identified
before a complete input snapshot exists.

### 7.2 `date` / `time`

`date` is parsed into `std::optional<eckit::Date>`.

`time` is parsed into `std::optional<eckit::Time>` only when present.

A missing `time` does not populate `marsTime`. `defaultMarsTime` is applied later
by anchor construction when a datetime needs to be materialized.

`time` without `date` is not rejected during low-level extraction unless the
input reader itself cannot represent it. It is rejected during
`TimeAnchorClassification`, because it is a cross-key temporal consistency
error.

### 7.3 `hdate`

`hdate` is parsed into `std::optional<eckit::Date>`.

There is no `htime` key in `ProductTimeSpec`. The time component for `hdate` is
always `defaultMarsTime`.

### 7.4 `year` / `month`

`year` and `month` must be both present or both absent.

`month` must be in `[1, 12]`. The pair must construct a valid
`eckit::Date(year, month, 1)`.

The direct reference anchor is:

```text
DateTime(Date(year, month, 1), defaultMarsTime)
```

The normalized input members are `marsYear` and `marsMonth`.

### 7.5 `step`

The raw `step` key is optional in `ProductTimeSpecInput`.

If present:

- it must be non-negative;
- positive values must be whole-hour aligned in the currently supported domain;
- positive sub-hourly or otherwise positive non-hour-aligned values are
  recognized but unsupported and rejected during extraction.

If absent:

- classification accepts it only for `marsType == "an"`;
- after successful classification, construction uses a resolved step of zero.

If `marsType == "an"` and `step` is explicitly present and non-zero, this is a
classification error.

### 7.6 `timespan`

`timespan` is normalized to `TimespanKind` plus optional seconds for duration
values.

- Missing key -> `TimespanKind::Missing`
- `"none"` -> `TimespanKind::None`
- `"fs"`, `"from-start"`, `"fromstart"` -> `TimespanKind::FromStart`
- supported duration -> `TimespanKind::Duration` and `timespanInSeconds`

Integer-valued `timespan` is interpreted as hours.

### 7.7 `stattype`

`stattype` is parsed into `ParsedStatTypeBlocks`.

The parser records both:

- the period-derived `timeRange`;
- the operation-derived `typeOfStatisticalProcessing`.

The current language whitelist may reject values with more than two blocks. The
parsed-block sequence and final canonical storage do not impose that structural
limit; both should avoid heap allocation in the currently supported domain.

### 7.8 `class`, `stream`, `type`

`class`, `stream`, and `type` are mandatory encoder-level MARS context keys and
are stored as normalized strings.

### 7.9 `timeIncrementInSeconds`

If present, `timeIncrementInSeconds` must be strictly positive.

It is a source/builder input. It is not stored directly in the final canonical
IR. Canonical windows store `ProductTimeDuration timeIncrement` instead.

### 7.10 `innerMostTypeOfStatisticalProcessing`

The caller supplies the innermost statistical-processing type.

It is used for:

- standard single-loop products;
- from-start single-loop products;
- the innermost window of old-style multi-loop products;
- FakeDoubleLoop consistency validation.

It is not used to populate outer `stattype` windows. Outer window types are read
from `stattype` itself.

For `FakeDoubleLoopSingleLoop`, there is one promoted `stattype` window and no
separate `timespan` window. The caller-supplied value must equal
`stattypeBlocks[0].typeOfStatisticalProcessing`; otherwise classification throws
a hard consistency error.

For instant products, this value must be `Missing`.

### 7.11 Options

The options snapshot contains:

- `allowDefaultTimeIncrementInSeconds`;
- `defaultTimeIncrementInSeconds`;
- `allowZeroLengthFsWindow`;
- `allowNonEnumeratedPositiveIntegerTimespanHours`;
- `allowRedundantTimeIncrement`;
- `allowMissingTimespanForInstantProduct`;
- `defaultTypeOfTimeIncrement`.

When `allowDefaultTimeIncrementInSeconds == true`,
`defaultTimeIncrementInSeconds` must be present and strictly positive. The value
is used only for eligible `DefaultedIncrement` branches. It must never repair a
missing increment for non-`ml` `FromStartSingleLoop`. When
`allowDefaultTimeIncrementInSeconds == false`, a missing source increment is not
repaired by defaulting.

`defaultTypeOfTimeIncrement` must be non-`Missing` whenever a real positive
increment is materialized, including both `ExplicitIncrement` and
`DefaultedIncrement`.

`allowRedundantTimeIncrement` applies only to branches where the final canonical
increment is semantically missing: instant products and AIFS-pure
single-window products. When disabled, an explicit source increment in those
branches is a hard classification error. When enabled, the explicit source value
is ignored and the final canonical increment remains the missing sentinel.

`allowMissingTimespanForInstantProduct` is a compatibility option. By
specification, instant products must use `timespan="none"`. A missing `timespan`
with no `stattype` blocks classifies as `Instant` only when this option is
explicitly enabled. When disabled, that source shape is a hard
shape-classification error.

All options used to construct the final object must be serializable in the final
`ProductTimeSpec` JSON payload.

---

## 8. Error Handling Contract

### 8.1 Layered Exception Policy

The exception policy has three layers:

1. low-level parsing / extraction failures without `ProductTimeSpecInput`;
2. internal `ProductTimeSpec` failures with input and optional classification
   payloads;
3. public deduction failures wrapping the internal exception chain.

### 8.2 Internal Exception Type

`Mars2GribProductTimeSpecException` is the internal exception type for failures
after `ProductTimeSpecInput` exists.

It carries:

- stage;
- reason;
- input JSON;
- classification JSON when available;
- construction-artifact JSON when available;
- final `ProductTimeSpec` JSON when available.

### 8.3 Public Deduction Boundary

The public `mars2grib` deduction boundary catches internal exceptions and wraps
them in `Mars2GribDeductionException`, preserving the nested context.

### 8.4 JSON Payload Rendering

`ProductTimeSpec` must provide a JSON serializer suitable for exception payloads
and diagnostics. The following example shows the minimum JSON payload for an Instant product.

The JSON payload must include at least:

```json
{
  "anchor": {
    "labelDateTime": "...",
    "initialConditionsDateTime": "...",
    "referenceDateTime": "...",
    "anchorType": "..."
  },
  "windowStartDateTime": "...",
  "windowEndDateTime": "...",
  "kind": "Instant",
  "incrementKind": "NoIncrement",
  "numberOfTimeRanges": 1,
  "timeRanges": [
    {
      "typeOfStatisticalProcessing": "Missing",
      "typeOfTimeIncrement": "Missing",
      "timeRange": { "unit": "Second", "length": 0 },
      "timeIncrement": { "unit": "Second", "length": 0 }
    }
  ],
  "options": {
    "allowDefaultTimeIncrementInSeconds": false,
    "defaultTimeIncrementInSeconds": null,
    "allowZeroLengthFsWindow": false,
    "allowNonEnumeratedPositiveIntegerTimespanHours": false,
    "allowRedundantTimeIncrement": false,
    "allowMissingTimespanForInstantProduct": false,
    "defaultTypeOfTimeIncrement": "Missing"
  }
}
```

The JSON serializer must serialize only valid canonical time ranges. It must not
serialize unused inline storage slots.

---

## 9. Backend Contracts

### 9.1 Point-In-Time Backend

The point-in-time backend may consume only products with:

```text
kind() == Instant
```

It must reject every non-`Instant` product unless a future explicit extension
adds another point-in-time representation.

The point-in-time backend consumes only:

```text
anchor
windowEndDateTime
```

It derives:

```text
forecastTime = windowEndDateTime - referenceDateTime
```

It does not consume canonical time ranges. In particular, it ignores the fake
zero-length instant window.

### 9.2 Statistics Backend

The statistics backend may consume only products with:

```text
kind() != Instant
```

It must reject `Instant`, even though instant products contain one fake
zero-length canonical window in the IR.

The statistics backend consumes:

```text
referenceDateTime
windowStartDateTime
windowEndDateTime
numberOfTimeRanges()
ProductTimeWindow range
```

It derives:

```text
forecastTime = windowStartDateTime - referenceDateTime
endOfOverallTimeInterval = windowEndDateTime
numberOfTimeRanges = spec.numberOfTimeRanges()
```

This specification supports only non-negative statistical `forecastTime`. The frontend/final
invariants therefore guarantee:

```text
referenceDateTime <= windowStartDateTime
```

The backend must defensively reject an object violating this contract.

It lowers canonical windows from AoS to the ecCodes SoA layout:

```cpp
for (std::size_t i = 0; i < spec.numberOfTimeRanges(); ++i) {
    const ProductTimeWindow& w = spec[i];

    typeOfStatisticalProcessing[i]     = w.typeOfStatisticalProcessing;
    typeOfTimeIncrement[i]             = w.typeOfTimeIncrement;
    indicatorOfUnitForTimeRange[i]     = w.timeRange.unit;
    lengthOfTimeRange[i]               = w.timeRange.length;
    indicatorOfUnitForTimeIncrement[i] = w.timeIncrement.unit;
    timeIncrement[i]                   = w.timeIncrement.length;
}
```

The statistics backend must not reparse raw `stattype`, raw `timespan`, raw
`step`, or raw `timeIncrementInSeconds`.

### 9.3 Backend Separation Rule

Backend dispatch by `kind()` is mandatory:

- `Instant` routes to the point-in-time backend;
- every non-`Instant` product routes to the statistics backend.

The two backends must remain separate. Shared code may exist only below the
canonical IR boundary for mechanical tasks such as unit conversion or datetime
arithmetic. Semantic decisions belong to the frontend and canonicalization
stages.

---

## 10. Worked Examples

### 10.1 Instant Product

Normative input shape:

```text
timespan = none
stattype missing
step = 12h
innerMostTypeOfStatisticalProcessing = Missing
```

Canonical result:

```text
kind = Instant
windowStartDateTime == windowEndDateTime
numberOfTimeRanges() == 1
windows[0] = Missing / Missing / 0 seconds / 0 seconds
```

Point-in-time lowering:

```text
forecastTime = windowEndDateTime - referenceDateTime
```

### 10.2 Compatibility Instant Product With Missing `timespan`

Compatibility input shape:

```text
timespan missing
stattype missing
allowMissingTimespanForInstantProduct = true
```

Canonical result is identical to the instant product above. If
`allowMissingTimespanForInstantProduct == false`, this input shape is a hard
shape-classification error.

### 10.3 Standard Single-Loop Statistic

Input shape:

```text
timespan = 6h
stattype missing
innerMostTypeOfStatisticalProcessing = Accumulation
timeIncrementInSeconds = 3600
```

Canonical windows:

```text
windows[0].timeRange = 6h
windows[0].typeOfStatisticalProcessing = Accumulation
windows[0].timeIncrement = 1h
windows[0].typeOfTimeIncrement = default non-missing policy value
```

### 10.4 Multi-Loop Statistic

Input shape:

```text
stattype = moav_damn
timespan = 6h
innerMostTypeOfStatisticalProcessing = Accumulation
timeIncrementInSeconds = 3600
```

Canonical windows:

```text
windows[0] = 1 month, Average,     increment = 1 day
windows[1] = 1 day,   Minimum,     increment = 6 hours
windows[2] = 6 hours, Accumulation, increment = 1 hour
```

Statistics lowering:

```text
forecastTime = windowStartDateTime - referenceDateTime
endOfOverallTimeInterval = windowEndDateTime
numberOfTimeRanges = 3
```

### 10.5 FakeDoubleLoop Statistic

Input shape:

```text
timespan = none
stattype = damn
innerMostTypeOfStatisticalProcessing = Minimum
(class, stream) requires fakeDoubleLoop
timeIncrementInSeconds = 3600
defaultTypeOfTimeIncrement = <non-Missing real-increment policy value>
```

Canonical windows:

```text
windows[0] = 1 day, Minimum, increment = 1 hour
```

There is no separate innermost `timespan` window. If the caller-supplied
`innerMostTypeOfStatisticalProcessing` differs from `Minimum`, the product is
rejected.

### 10.6 From-Start Statistic

Complete valid non-`ml` input shape:

```text
class = od
timespan = fs
step = 24h
innerMostTypeOfStatisticalProcessing = Accumulation
timeIncrementInSeconds = 3600
defaultTypeOfTimeIncrement = <non-Missing real-increment policy value>
```

Canonical result:

```text
windowStartDateTime = referenceDateTime
windowEndDateTime = referenceDateTime + 24h
windows[0] = 24h, Accumulation, increment = 1h
```

A missing `timeIncrementInSeconds` would be a hard error even if general
increment defaulting were enabled.

### 10.7 Zero-Length From-Start Statistic

Complete valid non-`ml` input shape:

```text
class = od
timespan = fs
step = 0
allowZeroLengthFsWindow = true
innerMostTypeOfStatisticalProcessing = Accumulation
timeIncrementInSeconds = 3600
defaultTypeOfTimeIncrement = <non-Missing real-increment policy value>
```

Canonical result:

```text
kind = FromStartSingleLoop
windowStartDateTime == windowEndDateTime == referenceDateTime
windows[0].timeRange = 0 seconds
windows[0].typeOfStatisticalProcessing = Accumulation
windows[0].timeIncrement = 1 hour
```

This is the only zero-length real statistical support allowed by policy. It is
also the only exception to `timeIncrement <= timeRange`.

### 10.8 AIFS-Pure Missing Increment

Input shape, ordinary case:

```text
class = ml
single real statistical window
timeIncrementInSeconds missing
```

Input shape, redundant-explicit case:

```text
class = ml
single real statistical window
timeIncrementInSeconds present and positive
allowRedundantTimeIncrement = true
```

If the redundant-explicit case occurs with `allowRedundantTimeIncrement = false`,
classification throws a hard error.

Canonical window in both accepted cases:

```text
typeOfTimeIncrement = Missing
timeIncrement = 0 seconds
```

`incrementKind() == AifsPureMissingIncrement`.

### 10.9 Point-In-Time vs Statistics `forecastTime`

For a statistical product:

```text
referenceDateTime = 00Z
windowStartDateTime = 06Z
windowEndDateTime = 12Z
```

The point-in-time formula, applied only as a counterfactual comparison, would
produce:

```text
forecastTime = 12h
```

The point-in-time backend must not actually consume this statistical object.

Valid statistics lowering uses:

```text
forecastTime = 6h
endOfOverallTimeInterval = 12Z
```

Therefore `forecastTime` is not a canonical field.

## 11. Test Plan

Tests must cover at least the following categories.

### 11.1 Input Extraction

- malformed date/time;
- `time` without `date` reaches anchor classification and is rejected there;
- missing `class`, `type`, or `stream`;
- partial `year` / `month`;
- invalid `month`;
- invalid `Date(year, month, 1)`;
- negative `step`;
- positive sub-hourly `step` rejected as unsupported;
- positive non-hour-aligned `step` rejected as unsupported;
- unsupported `timespan=inst`;
- unsupported sub-hourly `timespan`;
- malformed `stattype`;
- language-invalid `stattype`.

### 11.2 Classification

- normative instant from `timespan=none`;
- missing `timespan` with no `stattype` rejected when
  `allowMissingTimespanForInstantProduct == false`;
- missing `timespan` with no `stattype` accepted as compatibility instant when
  `allowMissingTimespanForInstantProduct == true`;
- standard single-loop;
- fakeDoubleLoop valid;
- fakeDoubleLoop required but represented as standard single-loop;
- fakeDoubleLoop representation used when not required;
- a genuine `MultiLoop` product is unaffected by the FakeDoubleLoop policy even
  when its `(class, stream)` pair appears in the required-representation list;
- FakeDoubleLoop caller processing equal to the processing parsed from
  `stattype`;
- FakeDoubleLoop caller processing mismatch rejected;
- `timespan=none` with more than one `stattype` block;
- from-start with `stattype` rejected;
- zero-length from-start allowed/disabled;
- analysis product missing `step` accepted;
- analysis product explicit non-zero `step` rejected;
- non-`ml` from-start with missing source increment rejected even when
  `allowDefaultTimeIncrementInSeconds == true`;
- defaulted increment accepted only for eligible non-`ml`, non-from-start
  products when a positive `defaultTimeIncrementInSeconds` and non-missing
  `defaultTypeOfTimeIncrement` are available;
- `ExplicitIncrement` / `DefaultedIncrement` rejected when
  `defaultTypeOfTimeIncrement == Missing`;
- frontend real-window counting for every shape;
- `StandardSingleLoop`, `MultiLoop`, and `FromStartSingleLoop` reject missing
  caller-provided innermost processing;
- every direct-source combination in the full anchor inheritance matrix;
- exact `TimeAnchorKind` for every direct-source combination;
- anchor ordering violation after inheritance.

### 11.3 Canonical Windows And Final Invariants

- instant fake window has both processing codes `Missing` and zero durations;
- standard single-loop creates exactly one real window;
- fakeDoubleLoop promotes the single `stattype` block and creates exactly one
  real window;
- from-start creates exactly one real window;
- multi-loop creates at least two real windows;
- malformed single-loop candidates with extra canonical windows are rejected;
- malformed multi-loop candidates with fewer than two canonical windows are
  rejected;
- multi-loop `moav_damn + timespan` creates three ordered windows;
- per-window `typeOfStatisticalProcessing` is read from `stattype` for outer
  windows;
- innermost `typeOfStatisticalProcessing` is the deduced/caller-supplied inner
  type;
- every real canonical window rejects
  `typeOfStatisticalProcessing == Missing`;
- the final support start equals `windowEndDateTime` minus the outermost
  canonical `timeRange` for every statistical product;
- outer-window `timeIncrement` equals immediately inner `timeRange`;
- innermost-window `timeIncrement` equals resolved increment;
- every real window satisfies `timeIncrement <= timeRange`;
- invalid multi-loop nesting where an inner range exceeds the outer range is
  rejected;
- zero-length from-start with a positive explicit increment is accepted as the
  unique exception;
- final zero-length from-start state is derived from `kind`, canonical window,
  support interval, reference time, and option snapshot; no final redundant
  boolean is required;
- `DefaultedIncrement` materializes `defaultTimeIncrementInSeconds` in the
  innermost real window;
- `AifsPureMissingIncrement` uses the missing-increment sentinel even when a
  redundant explicit source increment was ignored;
- statistical support beginning before `referenceDateTime` is rejected;
- JSON serializes only valid windows and reports
  `numberOfTimeRanges == timeRanges.size()`;
- exact duration canonicalization for `0s`, `3600s`, `6h`, `24h`, and `86400s`;
- `24h` remains elapsed-hour semantics unless explicitly calendar-derived;
- day/month calendar-alignment failures are rejected.

### 11.4 Backends

- point-in-time backend uses `windowEndDateTime` as `forecastTime` source;
- statistics backend uses `windowStartDateTime` as `forecastTime` source;
- statistics backend uses `windowEndDateTime` as `endOfOverallTimeInterval`;
- statistics backend rejects negative `forecastTime`;
- statistics backend lowers AoS canonical windows to SoA ecCodes arrays;
- point-in-time backend ignores the fake instant window;
- point-in-time backend rejects non-`Instant` products;
- statistics backend rejects `Instant` products.

### 11.5 Stage And Exception Contract

- `ProductTimeSpecConstruction` occurs before `FinalConsistencyCheck`;
- final-invariant failures carry a complete final candidate JSON payload when
  available;
- failures before a complete `ProductTimeSpecInput` exists use
  `Mars2GribGenericException`;
- internal later-stage failures use `Mars2GribProductTimeSpecException` with the
  correct stage.

### 11.6 Storage

- normal supported products allocate no dynamic window storage;
- overflow storage works for synthetic tests with more than
  `inlineProductTimeWindows` windows;
- read-only iteration covers inline and overflow windows;
- `operator[]`, `at()`, `size()`, and `numberOfTimeRanges()` are consistent.

---

## 12. Implementation Organization Notes

Suggested implementation organization:

```text
ProductTimeSpecInput.h/.cc
ProductTimeSpecClassification.h/.cc
ProductTimeSpecAnchor.h/.cc
ProductTimeSpecShape.h/.cc
ProductTimeSpecIncrement.h/.cc
ProductTimeDuration.h
ProductTimeWindow.h
ProductTimeWindows.h/.cc
ProductTimeSpec.h/.cc
ProductTimeSpecJson.h/.cc
ProductTimeSpecErrors.h/.cc
ProductTimeSpecClassifiers.h/.cc
ProductTimeSpecBuilders.h/.cc
ProductTimeSpecCanonicalize.h/.cc
```

Backend-specific lowering should remain outside the frontend/canonical IR module:

```text
backend/concepts/pointInTime/...
backend/concepts/statistics/...
```

The final canonical object should be cheap to copy or movable without heap
allocation in the normal case. Avoid `const` data members inside hot-path value
storage. Enforce immutability through private storage and const accessors.

`ProductTimeSpec` construction is hot-path code. The normal path must not depend
on heap allocation, repeated dictionary access, string parsing, or backend key
logic.
