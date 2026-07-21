# ProductTimeSpec Input Companion Specification

> Status: companion specification.
>
> This document is a focused companion to `productTimeSpecV3_final.md`. It does
> not replace the monolithic specification. Instead, it reorganizes the same
> ProductTimeSpec model from the perspective of input extraction and
> normalization so that the public input entry point and the `detail/input/`
> headers can be documented and maintained independently.

---

## 1. Scope

This companion document covers only the extraction and normalization boundary of
`ProductTimeSpec`.

It therefore focuses on:

- the normalized source snapshot `ProductTimeSpecInput`;
- lexical and representation parsing of temporal values;
- dictionary-facing normalization helpers;
- source-language parsing of `timespan` and `stattype`;
- option snapshot extraction;
- extraction-state assembly through `make...` helpers;
- diagnostic JSON serialization of normalized input state.

It does not define:

- time-anchor classification;
- shape classification;
- increment classification;
- cross-classification consistency checks;
- construction of `ProductTimeSpecAnchor`, `ProductTimeSpecShape`, or
  `ProductTimeSpecIncrement`;
- canonical `ProductTimeSpec` materialization.

Those topics belong to `ProductTimeSpecResolver_spec.md` and remain fully
defined in `productTimeSpecV3_final.md`.

---

## 2. Relationship To The Full Specification

This document reorganizes material already present in the full specification,
mainly from:

- Section 3.1: Inputs Used By `ProductTimeSpec`;
- Section 4.9: `ProductTimeSpecOptions`;
- Section 4.10: `ProductTimeSpecInput`;
- Section 5.3: duration canonicalization, where relevant to input parsing;
- Section 7: Input Extraction And Normalization;
- Section 8.1 and 8.4: extraction-layer error and JSON payload rules.

When this companion and the full specification overlap, the full specification
remains the canonical source of truth.

---

## 3. Responsibilities Of `ProductTimeSpecInput`

`ProductTimeSpecInput` is a normalized typed source snapshot. It exists to avoid
repeated dictionary access and to provide stable context for later exception
payloads.

Extraction owns:

- dictionary access through dictionary-access traits;
- lexical parsing of dates, times, durations, and option representations;
- case normalization of selected textual inputs;
- normalization of source types into stable typed fields;
- validation of individual extracted values and local option constraints;
- preservation of source absence through `std::optional`;
- diagnostic JSON serialization of normalized input state.

Extraction does not own:

- classification;
- cross-classification consistency;
- semantic construction artifacts;
- final canonical IR construction.

---

## 4. Input Sources

`ProductTimeSpec` is resolved from four source domains.

1. MARS dictionary

   Relevant keys:

   - `date`
   - `time`
   - `hdate`
   - `year`
   - `month`
   - `step`
   - `timespan`
   - `stattype`
   - `class`
   - `stream`
   - `type`

2. Parameter / misc dictionary

   Relevant key:

   - `timeIncrementInSeconds`

3. Option dictionary

   Relevant extracted option snapshot fields:

   - `allowDefaultTimeIncrementInSeconds`
   - `allowZeroLengthFsWindow`
   - `allowNonEnumeratedPositiveIntegerTimespanHours`
   - `allowRedundantTimeIncrement`
   - `allowMissingTimespanForInstantProduct`
   - `defaultTypeOfTimeIncrement`

4. Caller-supplied input

   - `innerMostTypeOfStatisticalProcessing`

The MARS context keys `class`, `stream`, and `type` are mandatory during
extraction even when a specific later branch uses only a subset of them.

---

## 5. Normalized Input State

The extraction result is a complete typed snapshot whose fields correspond to
stable normalized source values, not yet to final ProductTimeSpec semantics.

Conceptually:

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

- all fields are normalized and typed;
- `marsClass`, `marsStream`, and `marsType` are mandatory lowercase strings;
- `marsYear` and `marsMonth` are either both present or both absent;
- `stattypeBlocks` preserve parsed MARS textual order;
- the structure contains no classification results;
- the structure contains no final semantic fields such as canonical windows or
  support interval datetimes.

---

## 6. Extraction Pipeline

Extraction reads dictionaries once and returns `ProductTimeSpecInput` only if
all source-level normalized values are valid and supported.

A practical extraction order is:

1. parse and normalize the option snapshot;
2. read the mandatory MARS context triple (`class`, `stream`, `type`);
3. parse direct anchor sources (`date`, `time`, `hdate`);
4. parse and validate the `year` / `month` reference anchor pair;
5. parse `step`;
6. parse `timespan`;
7. parse `stattype`;
8. parse the explicit parameter-side increment.

This order ensures that any source-language rule depending on previously
normalized options can consult the complete option snapshot without rereading
the option dictionary.

---

## 7. Common Lexical Helpers

The input layer may provide small reusable lexical helpers in a common detail
header. Typical responsibilities are:

- lowercasing one token with byte-wise ASCII `std::tolower` semantics;
- removing one allowed separator character from a token;
- parsing one complete decimal integer token as `long`;
- converting hours to seconds with checked overflow behaviour.

These helpers do not themselves define ProductTimeSpec semantics. They support
the more specific temporal and source-language parsers.

---

## 8. Temporal Parsing

### 8.1 `date` / `time`

`date` is parsed into `std::optional<eckit::Date>`.

Accepted source representations may include:

- integer `YYYYMMDD`;
- string `YYYYMMDD`;
- string `YYYY-MM-DD`.

`time` is parsed into `std::optional<eckit::Time>` only when present.

Accepted source representations may include:

- compact numeric MARS time in `HHMM` form;
- already normalized numeric `HHMMSS` form;
- string `HHMM`, `HHMMSS`, `HH:MM`, or `HH:MM:SS`.

A missing `time` does not populate `marsTime`. `defaultMarsTime` is applied
later by anchor construction when a datetime must be materialized.

`time` without `date` is preserved during extraction and is rejected later by
`TimeAnchorClassification`, because it is a cross-key temporal consistency
error rather than a local lexical parsing error.

### 8.2 `hdate`

`hdate` is parsed into `std::optional<eckit::Date>`.

There is no `htime` input in ProductTimeSpec. The time component associated with
`hdate` is always `defaultMarsTime` and is applied later during anchor
construction.

### 8.3 `year` / `month`

`year` and `month` are optional, but they form one indivisible source:

- both must be present together;
- or both must be absent together.

If present:

- `month` must be in `[1,12]`;
- `eckit::Date(year, month, 1)` must be valid.

The normalized members are `marsYear` and `marsMonth`. Construction of a direct
reference-anchor datetime from them happens later.

### 8.4 `step`

The raw `step` key is optional in normalized input.

If present:

- it must be non-negative;
- positive values must be whole-hour aligned in the currently supported domain;
- positive sub-hourly or otherwise positive non-hour-aligned values are
  recognized but unsupported and rejected during extraction.

If absent, later classification accepts it only for `marsType == "an"`.

---

## 9. Dictionary Access And Type Normalization

Extraction may expose helpers for stable dictionary-facing normalization.

Typical rules include:

- optional integers accept either native integer storage or integer strings;
- optional strings do not perform implicit type conversion;
- mandatory strings are read with hard failure on absence or wrong type and are
  normalized to lowercase where required by ProductTimeSpec;
- boolean options accept an explicit small set of representations rather than
  generic truthiness;
- `defaultTypeOfTimeIncrement` may be represented symbolically or numerically.

These helpers sit at the boundary between raw heterogeneous dictionaries and the
typed ProductTimeSpec extraction layer.

---

## 10. Source-Language Parsing

### 10.1 `timespan`

`timespan` is normalized to one `TimespanKind` plus an optional positive seconds
 payload for duration-valued cases.

Supported normalized states are:

- missing key -> `TimespanKind::Missing`;
- `"none"` -> `TimespanKind::None`;
- `"fs"`, `"from-start"`, `"fromstart"` -> `TimespanKind::FromStart`;
- supported duration -> `TimespanKind::Duration` and `timespanInSeconds`.

Integer-valued `timespan` is interpreted as hours.

The current supported domain distinguishes between:

- explicitly supported values;
- recognized-but-unsupported values;
- invalid values.

Recognized-but-unsupported values should be rejected at extraction time with a
 diagnostic that distinguishes them from malformed input.

### 10.2 `stattype`

`stattype` is parsed into `ParsedStatTypeBlocks`.

Each parsed block records both:

- a period-derived `timeRange`;
- an operation-derived `typeOfStatisticalProcessing`.

The parser should preserve source block order. Current supported-domain policy
may reject values beyond the active whitelist, but that is a source-language
restriction, not a storage-model restriction.

---

## 11. Option Snapshot Extraction

The extracted option snapshot conceptually contains:

- `allowDefaultTimeIncrementInSeconds`;
- `allowZeroLengthFsWindow`;
- `allowNonEnumeratedPositiveIntegerTimespanHours`;
- `allowRedundantTimeIncrement`;
- `allowMissingTimespanForInstantProduct`;
- `defaultTypeOfTimeIncrement`.

The input layer normalizes representations and preserves a stable typed snapshot.
Later resolver stages decide which options are semantically exercised by the
current product.

---

## 12. Intermediate Extraction State And Make-Functions

An implementation may build ProductTimeSpecInput through one explicit
intermediate state object rather than mutating the final object during
extraction.

Typical structure:

- one private `ProductTimeSpecInputState` aggregate mirroring the final stored
  members;
- small `make...` helpers, each owning one local extraction responsibility;
- one top-level `makeProductTimeSpecInputState_or_throw(...)` orchestrator.

This pattern keeps responsibilities narrow:

- each `make...` function adds one local extraction context;
- the top-level state builder adds the stable outer extraction context;
- the public ProductTimeSpecInput constructor becomes a thin wrapper around
  state construction and final member materialization.

---

## 13. Diagnostic JSON Serialization

The input layer should provide a JSON representation of the complete normalized
input snapshot.

The JSON object includes:

- all normalized optional source fields;
- `timespanKind` and `timespanInSeconds`;
- parsed `stattype` blocks;
- mandatory MARS context strings;
- explicit parameter-side increment;
- caller-supplied innermost processing type;
- the normalized option snapshot.

Absent optionals are emitted as JSON `null`. Enums are emitted by symbolic name.
The JSON describes normalized state, not original source spelling or original
source storage types.

---

## 14. Extraction Error Handling

Extraction failures belong to the outer low-level ProductTimeSpec input layer.

Expected behaviour:

- local parsing and extraction helpers throw `Mars2GribGenericException` with a
  precise key-specific reason;
- the top-level extraction function or constructor catches and rethrows with a
  stable outer context such as `Failed to extract normalized ProductTimeSpec input`;
- later resolver stages use `Mars2GribProductTimeSpecException` only after a
  complete input snapshot exists.

---

## 15. Mapping To Code

This companion specification corresponds primarily to:

- `ProductTimeSpecInput.h`;
- `detail/input/ProductTimeSpecInputCommon.h`;
- `detail/input/ProductTimeSpecInputTemporalParsing.h`;
- `detail/input/ProductTimeSpecInputDictionaryAccess.h`;
- `detail/input/ProductTimeSpecInputSourceLanguage.h`;
- `detail/input/ProductTimeSpecInputOptions.h`;
- `detail/input/ProductTimeSpecInputStateAndMakers.h`;
- `detail/input/ProductTimeSpecInputToJson.h`.
