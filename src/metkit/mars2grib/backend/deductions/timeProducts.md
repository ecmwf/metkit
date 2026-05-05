# ProductTime — Specification

> Status: **normative**. This document supersedes all prior design notes for time
> handling in `mars2grib`. It contains no open questions and no historical
> commentary; every rule is prescriptive (`MUST` / `MUST NOT` / `MAY`).

---

## 1. Purpose and scope

`ProductTime` is the **single canonical representation** of all temporal
information associated with a MARS product within `mars2grib`.

The temporal consumers `referenceTime`, `pointInTime`, and `statistics` MUST
obtain their temporal data exclusively from a `ProductTime` instance produced by
`resolve_ProductTime_or_throw`. They MUST NOT reinterpret raw MARS keys
(`date`, `time`, `hdate`, `htime`, `fcyear`, `fcmonth`, `step`, `timespan`,
`stattype`) independently.

Out of scope for this document:
- The MARS-side parsing of request strings.
- The `paramId` → statistical-processing-type lookup table.
- The GRIB-side encoding of fields outside the temporal domain.
- The migration of consumer concept files (tracked separately).

---

## 2. Architecture

```
MARS  par  opt
  \   |   /
   \  |  /
    v v v
  resolve_ProductTime_or_throw   (deductions/productTime.h)
        |
        | builds ProductTimeInput, calls factory
        v
  makeProductTime_or_throw       (deductions/detail/productTime.h)
        |
        | validates invariants, returns immutable ProductTime
        v
  +----------------+
  |   ProductTime  |
  +----------------+
        |
        +--> referenceTime  consumer
        +--> pointInTime    consumer
        +--> statistics     consumer
                |
                +--> resolve_TypeOfStatisticalProcessing_or_throw
                |        (deductions/typeOfStatisticalProcessing.h, §23;
                |         self-contained, depends on detail/StatType.h, §22)
                +--> computeStatisticDescription(productTime, types)
                          (deductions/statisticsDescriptor.h)
```

`computeStatisticDescription` is a **pure function**: it depends only on its
arguments. No MARS / par / opt access at descriptor-build time.

---

## 3. `StatisticalWindow` model

`ProductTime` represents each statistical window as a `(unit, count)` pair,
reusing the existing GRIB-aligned `tables::TimeUnit` enum
(`backend/tables/timeUnits.h`, GRIB2 Code Table 4.4). No parallel "kind" enum
is introduced.

The `StatisticalWindow` type is defined in the shared header
`deductions/detail/StatisticalWindow.h` (§21) and is consumed by
`detail/ProductTime.h`, `detail/StatType.h` (the shared `stattype` parser,
§22), and `typeOfStatisticalProcessing.h` (§23).

```cpp
#include "metkit/mars2grib/backend/tables/timeUnits.h"

struct StatisticalWindow {
    tables::TimeUnit unit{tables::TimeUnit::Second};
    long             count{0};
};
```

### 3.1 Allowed `TimeUnit` subset

Although `tables::TimeUnit` enumerates the full GRIB2 code table, the only
values legal inside a `ProductTime::statisticalWindows` entry are:

```text
tables::TimeUnit::Second
tables::TimeUnit::Day
tables::TimeUnit::Month
```

The factory `makeProductTime_or_throw` MUST reject any other value (including
but not limited to `Minute`, `Hour`, `Hours3`, `Hours6`, `Hours12`, `Year`,
`Decade`, `Normal`, `Century`, `Missing`) as a hard error (§10.18).

This restriction reflects the locked production paths:
- `timespan` is always pre-converted to fixed seconds → produces
  `tables::TimeUnit::Second`;
- the locked `stattype` grammar (§7.7) emits only `mo` and `da` periods →
  produce `tables::TimeUnit::Month` and `tables::TimeUnit::Day` respectively.

Any future widening of this allow-list is an explicit spec amendment, not an
implementation freedom.

### 3.2 Invariants

- For any `StatisticalWindow` stored inside a `ProductTime` (i.e. inside
  `statisticalWindows[0 .. statisticalWindowCount)`), `count > 0` MUST hold.
- The factory rejects any input that contains a `StatisticalWindow` with
  `count <= 0` (hard error §10.11).
- The factory rejects any input that contains a `StatisticalWindow` with
  `unit` outside the §3.1 allow-list (hard error §10.18).
- The default-constructed `StatisticalWindow{}` has `count == 0`; it is legal
  at the C++ language level but illegal as a `ProductTime` member; it serves
  only as a temporary.

### 3.3 Calendar-vs-fixed classification

For the purposes of windowing semantics (§4) and calendar subtraction (§9.6),
the units in §3.1 are classified as:

| `unit`                       | Classification |
|------------------------------|----------------|
| `tables::TimeUnit::Second`   | fixed-duration |
| `tables::TimeUnit::Day`      | calendar-aligned |
| `tables::TimeUnit::Month`    | calendar-aligned |

The classification is implicit in the unit value; no separate field encodes
it.

### 3.4 Examples

```text
1h          -> StatisticalWindow{tables::TimeUnit::Second,  3600}
24h         -> StatisticalWindow{tables::TimeUnit::Second, 86400}
1 calendar day    -> StatisticalWindow{tables::TimeUnit::Day,   1}
1 calendar month  -> StatisticalWindow{tables::TimeUnit::Month, 1}
```

`tables::TimeUnit::Year` is intentionally excluded from the allow-list: it is
unreachable through the locked `stattype` grammar (§7.7) and not produced by
any other path.

---

## 4. Calendar-window semantics and strict alignment

### 4.1 Half-open interval convention

A product temporal support is the half-open interval `[windowStart, windowEnd)`,
**when** `windowStart < windowEnd`. The degenerate case `windowStart ==
windowEnd` denotes the **point in time** at `windowEnd` and is reserved for
instant products (§9.1).

### 4.2 Calendar day

A `StatisticalWindow{tables::TimeUnit::Day, N}` window is a window of N
**named** calendar days, internally represented as

```text
[YYYY-MM-DD 00:00:00, YYYY-MM-(DD+N) 00:00:00)
```

(with appropriate calendar carry into the next month / year).

Alignment rule for a `tables::TimeUnit::Day` outermost window:

```text
windowEnd MUST be at hh=00, mm=00, ss=00.
```

### 4.3 Calendar month

A `StatisticalWindow{tables::TimeUnit::Month, N}` window is a window of N
**named** calendar months. Its length depends on the concrete months
involved.

Alignment rule for a `tables::TimeUnit::Month` outermost window:

```text
windowEnd MUST be on day=1 at hh=00, mm=00, ss=00.
```

### 4.4 Strict alignment policy

When the outermost window in `statisticalWindows` is calendar-aligned (i.e.
its `unit` is `tables::TimeUnit::Day` or `tables::TimeUnit::Month`, per
§3.3), the factory MUST reject any input whose `windowEnd` violates the
corresponding alignment rule (hard errors §10.9, §10.10).

This is a deliberate behavioural change relative to legacy `timeUtils.h`-based
code, which silently tolerated misaligned ends. There is no opt-in flag and no
warn-only mode. Misaligned operational requests fail loudly and must be fixed
upstream.

---

## 5. `ProductTime` struct

```cpp
inline constexpr std::size_t maxStatisticalWindows = 3;

struct ProductTime {
    const eckit::DateTime simulationDateTime;
    const eckit::DateTime simulatedDateTime;
    const eckit::DateTime referenceDateTime;

    // Internal convention: [windowStart, windowEnd) when windowStart < windowEnd.
    // For instant products: windowStart == windowEnd (the point in time at windowEnd).
    const eckit::DateTime windowStart;
    const eckit::DateTime windowEnd;

    // Valid entries: statisticalWindows[0 .. statisticalWindowCount).
    // Ordering: outermost -> innermost.
    const std::array<StatisticalWindow, maxStatisticalWindows> statisticalWindows;
    const std::size_t                                          statisticalWindowCount;

    // Sampling increment of the innermost statistical loop.
    // std::nullopt for instant products (§9.1).
    // Optionally absent for single-window statistical products (AIFS path, §9.4).
    // Required and > 0 for multi-window statistical products.
    const std::optional<long> timeIncrementInSeconds;
};
```

All members are `const`. The struct is therefore immutable after construction,
copyable, and not assignable. `maxStatisticalWindows = 3` matches the maximum
producible by the locked `stattype` grammar (§7.6): one `mo` block + one `da`
block + one inner `timespan` window.

### 5.1 Tri-equivalent instant invariant

The factory MUST enforce, as a single equivalence:

```text
windowStart == windowEnd
   <=>
statisticalWindowCount == 0
   <=>
timeIncrementInSeconds == std::nullopt
```

i.e. all three conditions hold together or none does. Any other state is a
hard error (§10.5).

### 5.2 Window-end ordering invariant

```text
windowStart <= windowEnd     (always)
```

Strictly `<` for statistical products; equality reserved for instants.

### 5.3 Reference-vs-simulated invariant

```text
referenceDateTime >= simulatedDateTime     (always)
```

Violation is a hard error (§10.4). The corner case where reference < simulated
is officially out-of-spec MARS semantics and must be fixed upstream.

---

## 6. `ProductTimeInput`

The factory does not consume raw MARS keys. The resolver (§8) normalizes them
into `ProductTimeInput` first.

```cpp
enum class TimespanKind {
    Missing,    // MARS keyword absent
    Duration,   // MARS keyword has a duration value
    None        // MARS keyword equals literal "none" (fakeDoubleLoop, §9.4)
};

struct ProductTimeInput {
    eckit::DateTime simulationDateTime;

    std::optional<eckit::DateTime> simulatedDateTime;   // from hdate/htime
    std::optional<eckit::DateTime> referenceDateTime;   // from fcyear/fcmonth

    // Offset from referenceDateTime to ProductTime::windowEnd.
    long stepInSeconds{0};

    TimespanKind timespanKind{TimespanKind::Missing};

    // Valid only when timespanKind == TimespanKind::Duration.
    StatisticalWindow timespan{};

    // Temporal windows decoded from stattype (period part only),
    // ordered outermost -> innermost.
    std::array<StatisticalWindow, maxStatisticalWindows> stattypeWindows{};
    std::size_t                                          stattypeWindowCount{0};

    // Source: deductions::timeIncrementInSeconds_opt(mars, par) — i.e. par["timeIncrementInSeconds"].
    // Absent for instants and for the AIFS single-window path (§9.4).
    std::optional<long> timeIncrementInSeconds;
};
```

Factory:

```cpp
ProductTime makeProductTime_or_throw(const ProductTimeInput& input);
```

The factory:
1. Resolves defaults (simulated, reference) per §7.
2. Computes `windowEnd = referenceDateTime + stepInSeconds`.
3. Assembles `statisticalWindows` per the case table in §9.
4. Computes `windowStart` per §9.
5. Validates all invariants (§5.1, §5.2, §5.3) and all hard errors (§10).
6. Returns the immutable `ProductTime`.

---

## 7. MARS keywords and field-by-field resolution

### 7.1 Keyword reference

| Key        | Required             | Source                                       |
|------------|----------------------|----------------------------------------------|
| `date`     | conditional (§7.2)   | MARS                                         |
| `time`     | conditional (§7.2)   | MARS                                         |
| `hdate`    | no                   | MARS                                         |
| `htime`    | no                   | MARS                                         |
| `fcyear`   | no                   | MARS                                         |
| `fcmonth`  | no                   | MARS                                         |
| `step`     | no, default `0` (§7.5) | MARS                                       |
| `timespan` | no                   | MARS                                         |
| `stattype` | no                   | MARS                                         |
| `timeIncrementInSeconds` | no       | par (parameter dictionary), via `deductions::timeIncrementInSeconds_opt` |

### 7.2 `date` / `time`

Resolution rule for `simulationDateTime`:

```text
date present and time present:
    simulationDateTime := DateTime(date, time)

date missing and time missing and fcyear present and fcmonth present:
    simulationDateTime := DateTime(Date(fcyear, fcmonth, 1), Time(00:00:00))
    (this is the same value §7.4 produces for referenceDateTime;
     consequently simulationDateTime == referenceDateTime in this case)

any other combination -> hard error (§10.1)
```

`simulationDateTime` is a label for the simulation; it is not used for `step`
arithmetic. The `hdate`/`htime` rules in §7.3 are unchanged regardless of
whether the R2 default in §7.2 is taken: if `hdate` is present it still
drives `simulatedDateTime`, and the §5.3 invariant
`referenceDateTime >= simulatedDateTime` continues to apply (it may now
constrain `(fcyear, fcmonth, 1)` against `hdate`).

### 7.3 `hdate` / `htime`

```text
hdate missing,  htime missing  ->  simulatedDateTime := simulationDateTime
hdate present,  htime missing  ->  simulatedDateTime := DateTime(hdate, 00:00:00)
hdate present,  htime present  ->  simulatedDateTime := DateTime(hdate, htime)
hdate missing,  htime present  ->  hard error (§10.2)
```

### 7.4 `fcyear` / `fcmonth`

Both must be present together or both absent.

```text
both missing  ->  referenceDateTime := simulatedDateTime
both present  ->  referenceDateTime := DateTime(Date(fcyear, fcmonth, 1), Time(00:00:00))
exactly one present -> hard error (§10.3)
```

The day component is `1` (Gregorian-valid). No day-zero variant exists.

### 7.5 `step`

`step` is optional with default `0`.

```text
step missing  ->  stepInSeconds := 0
step present  ->  stepInSeconds := toSeconds_or_throw(step)
```

`toSeconds_or_throw` (signature locked from existing `detail/timeUtils.h`)
parses:

- bare numeric `N` as **N hours**;
- suffixed numeric with `h` (hours), `m` (minutes), `s` (seconds), `d` (days).

Then:

```text
windowEnd := referenceDateTime + stepInSeconds
```

`step` missing is observationally indistinguishable from `step = 0`; both
produce the same `ProductTime`. The internal type is `long`, not
`std::optional<long>`.

Whether the resulting `ProductTime` is then a legal instant or a misaligned
calendar window is determined by the case table (§9) and the alignment rule
(§4.4) — no special-case logic for the absent / zero step.

### 7.6 `timespan`

Three states (`TimespanKind`):

- **`Missing`** — keyword absent.
- **`Duration`** — keyword carries a duration. Parsed by `toSeconds_or_throw`
  (same rules as `step`, §7.5). The result is wrapped in
  `StatisticalWindow{tables::TimeUnit::Second, seconds}`. **Note**: in
  production the resolver always converts `timespan` to fixed seconds before
  entering the factory; the spec deliberately leaves no path for
  calendar-aligned `timespan`. If future needs require it, this section is
  the single point that must be amended.
- **`None`** — keyword equals the literal string `"none"`. Valid only in the
  fakeDoubleLoop case (§9.4); any other use is a hard error (§10.6, §10.7).

### 7.7 `stattype`

Grammar (locked, identical to existing `detail/timeUtils.h::parseStatType_or_throw`):

```text
stattype := block ('_' block)*
block    := period operation
period   := 'mo' | 'da'
operation := 'av' | 'mn' | 'mx' | 'sd'
```

Block length is exactly 4 characters; separator is exactly one underscore.

Semantic constraints (also locked from existing parser):

- at most one `mo` block;
- at most one `da` block;
- if both present, `mo` MUST precede `da` (outermost-to-innermost order).

The `ProductTime` factory uses the **period prefix only** for window assembly:

```text
period 'mo'  ->  StatisticalWindow{tables::TimeUnit::Month, 1}
period 'da'  ->  StatisticalWindow{tables::TimeUnit::Day,   1}
```

The operation suffix is consumed by `resolveTypeOfStatisticalProcessing` (a
sibling deduction, out of scope here). The `stattype` parser MUST be a single
shared helper (currently `detail/timeUtils.h::parseStatType_or_throw`,
relocated to `detail/productTime.h`) used by both deductions, so that the two
never drift.

### 7.8 `timeIncrementInSeconds`

Source: `deductions::timeIncrementInSeconds_opt(mars, par)`, which reads
`par["timeIncrementInSeconds"]`. Existing normalization is preserved:

- absent             -> `std::nullopt`
- present, value `0` -> `std::nullopt` (legacy normalization)
- present, value < 0 -> hard error (§10.10)
- present, value > 0 -> the value

The forwarded `std::optional<long>` becomes `ProductTime::timeIncrementInSeconds`
subject to the case table (§9) and the tri-equivalent invariant (§5.1).

---

## 8. Resolver

```cpp
template <class MarsDict_t, class ParDict_t, class OptDict_t>
ProductTime resolve_ProductTime_or_throw(
    const MarsDict_t& mars,
    const ParDict_t&  par,
    const OptDict_t&  opt);
```

- `opt` is accepted for signature consistency with sibling deductions but
  reads no keys at present. Reserved for future options.
- The resolver:
  1. Reads MARS keys per §7.
  2. Reads `par["timeIncrementInSeconds"]` via the existing
     `timeIncrementInSeconds_opt` helper.
  3. Builds a `ProductTimeInput`.
  4. Calls `makeProductTime_or_throw`.
  5. On success, emits exactly one `MARS2GRIB_LOG_RESOLVE` line (§12).
  6. On failure, rethrows-with-nested per §11.

`resolve_ProductTime_or_throw` is the canonical name. Any additional spelling
(e.g. `resolveProductTime_or_throw`) is a deprecated alias and SHOULD NOT be
used in new code.

Header location: `deductions/productTime.h` (public). The factory
`makeProductTime_or_throw` called by this resolver is declared in
`deductions/detail/productTime.h`. Both files share the basename
`productTime.h`; they are distinguished by the `detail/` subdirectory per the
codebase convention for implementation-detail headers (matching the existing
`detail/timeUtils.h` precedent and consistent with the convention that public
deduction files are named after the concept they produce).

---

## 9. Window-assembly cases

Four mutually-exclusive cases, distinguished by `timespanKind` and presence of
`stattype` blocks. Each row of the table specifies the **complete** state of
the resulting `ProductTime`.

| Case | `timespanKind` | `stattype` blocks | `statisticalWindows` (out→in) | `windowStart` | `timeIncrementInSeconds` |
|------|----------------|-------------------|-------------------------------|---------------|--------------------------|
| §9.1 Instant            | `Missing` | 0          | (empty)                                          | `windowEnd`                                  | `std::nullopt` (required) |
| §9.2 Old single-loop    | `Duration`| 0          | `[timespan]`                                     | `windowEnd - timespan`                       | optional or required (§9.5) |
| §9.3 Old multi-loop     | `Duration`| 1 or 2     | `[parse(stattype) ..., timespan]`                | `windowEnd - statisticalWindows[0]`          | required and > 0 |
| §9.4 New fakeDoubleLoop | `None`    | exactly 1  | `[parse(stattype)]`                              | `windowEnd - statisticalWindows[0]`          | optional or required (§9.5) |
| any other combination   | —         | —          | —                                                | —                                            | hard error (§10.6, §10.7, §10.8) |

In all rows, `simulationDateTime`, `simulatedDateTime`, `referenceDateTime`,
and `windowEnd` are computed per §7. The strict-alignment rule (§4.4) is
applied after `windowStart` is computed.

### 9.1 Instant product

```text
windowEnd               := referenceDateTime + stepInSeconds
windowStart             := windowEnd
statisticalWindows      := (empty array, statisticalWindowCount = 0)
timeIncrementInSeconds  := std::nullopt  (MUST; else §10.5)
```

Used primarily by `referenceTime` and `pointInTime`. `statistics` MUST NOT be
invoked on an instant `ProductTime`.

### 9.2 Old-style single-loop statistic

```text
windowEnd               := referenceDateTime + stepInSeconds
statisticalWindows      := [timespan]
statisticalWindowCount  := 1
windowStart             := windowEnd - timespan          (= windowEnd - timespan.count seconds)
```

`timespan.unit` is `tables::TimeUnit::Second` (§7.6). `windowStart` is
therefore a simple seconds subtraction; no calendar arithmetic is involved on
this path.

### 9.3 Old-style multi-loop statistic

```text
windowEnd               := referenceDateTime + stepInSeconds
statisticalWindows      := [ <stattype windows in outermost-to-innermost order> , timespan ]
statisticalWindowCount  := stattypeWindowCount + 1
windowStart             := windowEnd - statisticalWindows[0]
                            (calendar subtraction; see §9.6)
```

### 9.4 New-style fakeDoubleLoop ("`timespan = none`")

A deliberately redundant encoding that exists in production. Single statistical
loop only. The single `stattype` block carries both:

- a **period** (`mo` or `da`) consumed by `ProductTime`;
- an **operation** (`av` / `mn` / `mx` / `sd`) consumed by
  `resolveTypeOfStatisticalProcessing`.

The same statistical operation is also implied by `paramId`. The two MUST
agree; disagreement is a hard error (§10.12). This contract is enforced inside
the deduction `resolve_TypeOfStatisticalProcessing_or_throw` (§23), which
receives the `paramId`-derived operation as its
`innerTypeOfStatisticalProcessing` argument and compares it against the parsed
`stattype` block operation in this case.

```text
windowEnd               := referenceDateTime + stepInSeconds
statisticalWindows      := [parse(single stattype block)]
statisticalWindowCount  := 1
windowStart             := windowEnd - statisticalWindows[0]
                            (calendar subtraction; see §9.6)
```

### 9.5 `timeIncrementInSeconds` per case

| Case | Required value of `timeIncrementInSeconds` |
|------|--------------------------------------------|
| §9.1 Instant            | MUST be `std::nullopt`                                  |
| §9.2 Old single-loop    | MAY be `std::nullopt` (AIFS path); if present MUST be > 0 |
| §9.3 Old multi-loop     | MUST be present and > 0                                  |
| §9.4 New fakeDoubleLoop | MAY be `std::nullopt` (AIFS path); if present MUST be > 0 |

Violation is a hard error (§10.5 for instants, §10.10 for non-positive,
§10.13 for missing-where-required).

### 9.6 Calendar subtraction

For a window with `unit == tables::TimeUnit::Day` and `count == N`:

```text
windowStart := DateTime(windowEnd.date() - N days, 00:00:00)
```

For a window with `unit == tables::TimeUnit::Month` and `count == N`:

```text
windowStart := DateTime(Date(year, month, 1) shifted back N months, 00:00:00)
```

For a window with `unit == tables::TimeUnit::Second` and `count == N`:

```text
windowStart := windowEnd - N seconds   (no calendar arithmetic)
```

The calendar paths presuppose `windowEnd` already satisfies the corresponding
alignment rule (§4); the factory verifies alignment before performing the
subtraction.

---

## 10. Hard errors

The factory `makeProductTime_or_throw` and the resolver
`resolve_ProductTime_or_throw` MUST throw `Mars2GribDeductionException` on any
of the following conditions. Each entry corresponds to exactly one check site
in the implementation.

| #     | Condition                                                        |
|-------|------------------------------------------------------------------|
| 10.1  | `date` or `time` missing without fallback: `date` missing OR `time` missing, **except** when both `date` AND `time` are missing AND both `fcyear` AND `fcmonth` are present (in which case §7.2 substitutes the default). Note: `step` missing is NOT an error (§7.5 default 0). |
| 10.2  | `htime` present without `hdate`                                  |
| 10.3  | exactly one of `fcyear` / `fcmonth` present (must be both or neither) |
| 10.4  | `referenceDateTime < simulatedDateTime`                          |
| 10.5  | tri-equivalence broken: `windowStart == windowEnd` XOR `statisticalWindowCount == 0` XOR `timeIncrementInSeconds == nullopt` |
| 10.6  | `stattype` present but `timespan` missing                        |
| 10.7  | `timespan = none` but `stattype` missing                         |
| 10.8  | `timespan = none` with more than one `stattype` block            |
| 10.9  | outermost window has `unit == tables::TimeUnit::Day` and `windowEnd` is not at hh=00,mm=00,ss=00 |
| 10.10 | outermost window has `unit == tables::TimeUnit::Month` and `windowEnd` is not on day=1 at hh=00,mm=00,ss=00 |
| 10.11 | any `StatisticalWindow` in `statisticalWindows[0..count)` has `count <= 0` |
| 10.12 | (deduction-side, `resolve_TypeOfStatisticalProcessing_or_throw`, §23) in the §9.4 fakeDoubleLoop case, the parsed `stattype` block operation disagrees with the `innerTypeOfStatisticalProcessing` argument supplied by the caller |
| 10.13 | `statisticalWindowCount >= 2` and `timeIncrementInSeconds == nullopt` |
| 10.14 | `timeIncrementInSeconds` raw input value < 0                     |
| 10.15 | `statisticalWindowCount > maxStatisticalWindows` (= 3)           |
| 10.16 | `stattype` block parsed with unknown period or operation token (raised by the shared parser, §22; period MUST be in `{mo, da}` and operation MUST be in `{av, mn, mx, sd}`) |
| 10.17 | `stattype` blocks not in outermost-to-innermost order (e.g. `da_mo`) (raised by the shared parser, §22) |
| 10.18 | any `StatisticalWindow` in `statisticalWindows[0..count)` has `unit` outside the §3.1 allow-list `{Second, Day, Month}` (e.g. `Hour`, `Hours6`, `Year`, `Missing`). **Two-level enforcement**: (a) the shared parser (§22) enforces the narrow `stattype`-grammar allow-list `{Day, Month}` at parse time; (b) the factory `makeProductTime_or_throw` enforces the extended assembled-window allow-list `{Second, Day, Month}` after window assembly (the `Second` extension covers the innermost window when it originates from `timespan` rather than `stattype`). |

The tri-equivalence check (10.5) subsumes several otherwise-separate checks
(e.g. "instant with non-null increment", "statistical with zero-length
window"); they are aggregated into a single invariant for clarity.

---

## 11. Error contract

All failures use `Mars2GribDeductionException` with `Here()` for source
location. The pattern matches sibling deductions
(e.g. `numberOfTimeRanges.h`):

```cpp
try {
    /* internal logic, possibly throwing other deductions' exceptions */
}
catch (...) {
    std::throw_with_nested(
        Mars2GribDeductionException("Unable to resolve ProductTime", Here()));
}
```

Every check in §10 throws a short imperative message at its precise check
site. The outer wrapper adds context. Inner exceptions form a chain visible at
the catch boundary.

No other exception type is introduced. No subclassing per failure category.

---

## 12. Logging contract

`resolve_ProductTime_or_throw` MUST emit exactly **one** log line on
successful completion, via `MARS2GRIB_LOG_RESOLVE`. The payload is built
inline (no `operator<<` is provided; see §13) and lists every resolved field
in a stable, greppable form.

Indicative payload shape:

```text
`ProductTime` resolved from input dictionaries: simulationDateTime='...' \
simulatedDateTime='...' referenceDateTime='...' windowStart='...' windowEnd='...' \
statisticalWindowCount='N' statisticalWindows=['...','...'] timeIncrementInSeconds='...|missing'
```

No log emissions on intermediate sub-steps. No log emissions on failure
(failures travel via the exception chain).

---

## 13. Serialization

No `operator<<` is provided for `ProductTime` or `StatisticalWindow`. Both
the RESOLVE log line (§12) and any inline error-message text build their
string representation locally, in the same lambda style as
`numberOfTimeRanges.h`. Tests assert on individual fields rather than on
whole-struct equality.

---

## 14. Thread-safety

- `ProductTime` is immutable and trivially safe to share across threads.
- `makeProductTime_or_throw` and `resolve_ProductTime_or_throw` access no
  shared mutable state. Concurrent invocation on **disjoint** inputs is safe.
- Concurrent invocation that shares the same `MarsDict_t` / `ParDict_t` /
  `OptDict_t` instances is safe iff those dictionary types' read operations
  are themselves thread-safe (this is the dictionary author's contract, not
  this module's).

---

## 15. Per-consumer field-access table

The following table is **normative**: each consumer MUST read only the fields
marked `R` and MUST NOT read any field marked `—`. The table is not enforced at
the language level; reviewers and consumer-side tests are responsible.

| Field                       | `referenceTime` | `pointInTime` | `statistics` |
|-----------------------------|:---------------:|:-------------:|:------------:|
| `simulationDateTime`        | R               | —             | R            |
| `simulatedDateTime`         | R               | —             | R            |
| `referenceDateTime`         | R               | R             | R            |
| `windowStart`               | —               | —             | R            |
| `windowEnd`                 | —               | R             | R            |
| `statisticalWindows`        | —               | —             | R            |
| `statisticalWindowCount`    | —               | —             | R            |
| `timeIncrementInSeconds`    | —               | —             | R            |

The exact mapping between the legacy deductions used today by each consumer
(`resolve_ForecastTimeInSeconds_or_throw`, `resolve_ReferenceDateTime_or_throw`,
`resolve_HindcastDateTime_or_throw`, `resolve_SignificanceOfReferenceTime_or_throw`,
`timeIncrementInSeconds_opt`, etc.) and the `ProductTime` field accesses above
is part of the consumer-migration step (out of scope for this document).

---

## 16. Test plan

The implementation MUST ship the following tests, alongside the existing
`tests/mars2grib/...` layout (exact harness to match the in-place style).

### 16.1 Unit tests — happy paths

One test per worked example in §17. All §17 examples use concrete numeric
values, so each maps directly to a fixture.

### 16.2 Unit tests — hard errors

One negative test per entry in §10. Each test name MUST cite the §10 entry
number for traceability.

### 16.3 Operational regression sweep (pre-merge gate)

A curated set of real MARS requests is processed by both the legacy
`timeUtils.h`-based pipeline and the new `ProductTime` pipeline; outputs are
compared at the GRIB byte level.

- Requests that today produce **misaligned** calendar windows are EXPECTED to
  fail under the new pipeline (§4.4). Such failures are flagged in the test
  set, triaged, and forwarded upstream — they are not regressions.
- All other requests MUST produce **bit-identical** GRIB output. Any byte
  difference is a regression that MUST be fixed before merge.

The fixture loader is a placeholder; populating it with operational request
samples is an environment task, not a source-code task.

---

## 17. Worked examples

All values are concrete; each example is a unit-test fixture (§16.1).

### 17.1 Instant forecast product

Input:

```text
date     = 20260501
time     = 000000
step     = 24
timespan = (missing)
stattype = (missing)
```

Output:

```text
simulationDateTime      = 2026-05-01 00:00:00
simulatedDateTime       = 2026-05-01 00:00:00
referenceDateTime       = 2026-05-01 00:00:00
windowEnd               = 2026-05-02 00:00:00
windowStart             = 2026-05-02 00:00:00
statisticalWindows      = []
statisticalWindowCount  = 0
timeIncrementInSeconds  = std::nullopt
```

### 17.2 Hindcast/reforecast product

Input:

```text
date  = 20260501
time  = 000000
hdate = 19930501
htime = (missing)
step  = 24
```

Output:

```text
simulationDateTime      = 2026-05-01 00:00:00
simulatedDateTime       = 1993-05-01 00:00:00
referenceDateTime       = 1993-05-01 00:00:00
windowEnd               = 1993-05-02 00:00:00
windowStart             = 1993-05-02 00:00:00
statisticalWindows      = []
statisticalWindowCount  = 0
timeIncrementInSeconds  = std::nullopt
```

### 17.3 Climate / reforecast anchor

Input:

```text
date    = 20260501
time    = 000000
hdate   = 19930501
fcyear  = 1993
fcmonth = 5
step    = 24
```

Output:

```text
simulationDateTime      = 2026-05-01 00:00:00
simulatedDateTime       = 1993-05-01 00:00:00
referenceDateTime       = 1993-05-01 00:00:00     # Date(1993, 5, 1)
windowEnd               = 1993-05-02 00:00:00
windowStart             = 1993-05-02 00:00:00
```

Invariant §5.3 holds: `referenceDateTime == simulatedDateTime`.

### 17.4 Old-style single-loop statistic — hourly accumulation over 1h

Input:

```text
date     = 20260501
time     = 000000
step     = 24
timespan = 1h
stattype = (missing)
timeIncrementInSeconds (par) = 3600
```

Output:

```text
windowEnd               = 2026-05-02 00:00:00
windowStart             = 2026-05-01 23:00:00
statisticalWindows      = [StatisticalWindow{tables::TimeUnit::Second, 3600}]
statisticalWindowCount  = 1
timeIncrementInSeconds  = 3600
```

### 17.5 Old-style multi-loop — monthly average of daily minimum of hourly accumulation

Input (concrete values; replaces the previous textual placeholder):

```text
date     = 20260501
time     = 000000
step     = 744          # 31 days * 24 hours, end of May 2026
timespan = 1h
stattype = moav_damn
timeIncrementInSeconds (par) = 3600
```

Output:

```text
referenceDateTime       = 2026-05-01 00:00:00
windowEnd               = 2026-06-01 00:00:00     # day=1, alignment satisfied (§4.3)
statisticalWindows      = [
    StatisticalWindow{tables::TimeUnit::Month,  1},   # outermost
    StatisticalWindow{tables::TimeUnit::Day,    1},
    StatisticalWindow{tables::TimeUnit::Second, 3600} # innermost (= timespan)
]
statisticalWindowCount  = 3
windowStart             = 2026-05-01 00:00:00     # windowEnd minus 1 calendar month
timeIncrementInSeconds  = 3600
```

### 17.6 New-style fakeDoubleLoop — monthly average

Input:

```text
date     = 20260501
time     = 000000
step     = 744
timespan = none
stattype = moav
timeIncrementInSeconds (par) = 86400
paramId-implied operation    = average    # required to match 'av' (§9.4, §10.12); enforced inside resolve_TypeOfStatisticalProcessing_or_throw (§23)
```

Output:

```text
referenceDateTime       = 2026-05-01 00:00:00
windowEnd               = 2026-06-01 00:00:00     # alignment satisfied
statisticalWindows      = [StatisticalWindow{tables::TimeUnit::Month, 1}]
statisticalWindowCount  = 1
windowStart             = 2026-05-01 00:00:00
timeIncrementInSeconds  = 86400
```

### 17.7 Invalid new-style multi-loop — must throw

Input:

```text
timespan = none
stattype = moav_damn
```

Result: hard error per §10.8.

### 17.8 Analysis-time product (R1: step missing)

Input:

```text
date     = 20260501
time     = 000000
step     = (missing)
timespan = (missing)
stattype = (missing)
```

Output:

```text
simulationDateTime      = 2026-05-01 00:00:00
simulatedDateTime       = 2026-05-01 00:00:00
referenceDateTime       = 2026-05-01 00:00:00
windowEnd               = 2026-05-01 00:00:00     # step defaults to 0
windowStart             = 2026-05-01 00:00:00
statisticalWindows      = []
statisticalWindowCount  = 0
timeIncrementInSeconds  = std::nullopt
```

### 17.9 Default date/time from fcyear/fcmonth (R2)

Input:

```text
date     = (missing)
time     = (missing)
fcyear   = 1993
fcmonth  = 5
step     = (missing)
```

Output:

```text
simulationDateTime      = 1993-05-01 00:00:00     # defaulted from (fcyear, fcmonth, 1, 00:00:00)
simulatedDateTime       = 1993-05-01 00:00:00     # from simulationDateTime, hdate absent
referenceDateTime       = 1993-05-01 00:00:00     # from (fcyear, fcmonth, 1, 00:00:00)
windowEnd               = 1993-05-01 00:00:00     # step defaults to 0
windowStart             = 1993-05-01 00:00:00
statisticalWindows      = []
statisticalWindowCount  = 0
timeIncrementInSeconds  = std::nullopt
```

All three datetime fields collapse to the same value — the natural
consequence of the defaults.

---

## 18. Migration & cleanup plan

### 18.1 Files added

- `src/metkit/mars2grib/backend/deductions/productTime.h` — public deduction
  header, exposing `resolve_ProductTime_or_throw`. Named after the concept
  produced (`ProductTime`), consistent with sibling deduction files in this
  directory. Lowercase initial per the §20 naming convention
  (function-primary).
- `src/metkit/mars2grib/backend/deductions/detail/ProductTime.h` — types
  (`TimespanKind`, `ProductTimeInput`, `ProductTime`), factory
  (`makeProductTime_or_throw`), and helpers (calendar arithmetic, alignment
  checks, signed-second shifts). UpperCamelCase initial per the §20 naming
  convention (type-primary). The shared `StatisticalWindow` type is no longer
  defined here; it lives in `detail/StatisticalWindow.h` (§21). The shared
  `stattype` parser is no longer defined here; it lives in `detail/StatType.h`
  (§22).
- `src/metkit/mars2grib/backend/deductions/detail/StatisticalWindow.h` — shared
  header exposing the `StatisticalWindow` type (§21).
- `src/metkit/mars2grib/backend/deductions/detail/StatType.h` — shared
  `stattype` parser exposing `parse_StatType_or_throw` and the
  `ParsedStatTypeBlock` type (§22).
- `src/metkit/mars2grib/backend/deductions/typeOfStatisticalProcessing.h` —
  public deduction header, exposing `resolve_TypeOfStatisticalProcessing_or_throw`
  (§23). Lowercase initial per the §20 naming convention (function-primary).
- Unit tests per §16.1 and §16.2.

### 18.2 Files rewritten

- `src/metkit/mars2grib/backend/deductions/statisticsDescriptor.h` — becomes
  the pure function `computeStatisticDescription(productTime,
  typeOfStatisticalProcessing)`. The existing `StatisticalProcessing` struct
  is preserved. The current `reserve(...)` bug (vectors are indexed without
  being resized) is fixed by switching to `resize(...)`.

### 18.3 Files deleted

The following deductions are subsumed by `ProductTime` and removed:

- `src/metkit/mars2grib/backend/deductions/detail/timeUtils.h`
- `src/metkit/mars2grib/backend/deductions/forecastTimeInSeconds.h`
- `src/metkit/mars2grib/backend/deductions/timeSpanInSeconds.h`
- `src/metkit/mars2grib/backend/deductions/timeIncrementInSeconds.h`
- `src/metkit/mars2grib/backend/deductions/numberOfTimeRanges.h`
- `src/metkit/mars2grib/backend/deductions/referenceDateTime.h`
- `src/metkit/mars2grib/backend/deductions/hindcastDateTime.h`

`significanceOfReferenceTime.h` is removed **conditionally**: if its logic is
purely temporal it folds into the new pipeline; if it carries non-temporal
semantics (type/stream/etc.) it is preserved untouched. Determined by direct
inspection at code-generation time.

### 18.4 Files NOT modified in the deductions step

- `src/metkit/mars2grib/backend/concepts/point-in-time/pointInTimeEncoding.h`
- `src/metkit/mars2grib/backend/concepts/reference-time/referenceTimeEncoding.h`
- `src/metkit/mars2grib/backend/concepts/statistics/statisticsEncoding.h`

These three consumer files reference deductions that are deleted in §18.3 and
will therefore **fail to compile** after the deductions step. This is
intentional and matches the explicit scoping of the deductions-only work
package. A separate consumer-migration step (out of scope here) restores the
build by routing all temporal accesses through `resolve_ProductTime_or_throw`
and the per-consumer field-access table (§15).

A `STEP4_CONSUMER_MIGRATION_CHECKLIST.md` MUST be produced alongside the
deductions step, listing for each consumer file the exact symbol replacements
required.

### 18.5 Build system

`CMakeLists.txt` (or equivalent) entries that reference any deleted file are
updated in the same change set as §18.3.

### 18.6 Breaking-change call-out

The strict alignment rule (§4.4) is a behavioural change. Operational requests
that today silently produce misaligned calendar windows will start throwing.
This is the intended improvement; it MUST be flagged in release notes.

---

## 19. Locked-decision cross-reference

Each numbered design decision from the Step 0 / Step 1 design discussion maps
to one or more sections of this specification. The cross-reference is
maintained for traceability; future amendments SHOULD update both columns.

| Decision | Section(s)                |
|----------|---------------------------|
| A1 (timespan=none semantics)              | §6, §7.6, §9.4, §10.7, §10.8 |
| A2 / B8 (strict calendar alignment)       | §4.4, §10.9, §10.10          |
| A3 (reference >= simulated)               | §5.3, §10.4                  |
| A4 / C6 (instant tri-equivalence)         | §4.1, §5.1, §9.1, §10.5      |
| A5 / B1 (timeIncrementInSeconds in PT)    | §5, §6, §7.8, §9.5           |
| Q1 (fakeDoubleLoop paramId↔stattype, deduction-side) | §9.4, §10.12, §23 |
| B2 / C9 (stattype grammar)                | §7.7, §10.16, §10.17, §22    |
| B3 (bare numeric units = hours)           | §7.5, §7.6                   |
| B4 (Date(fcyear, fcmonth, 1))             | §7.4                         |
| B5 (reuse Period/StatOp/StatTypeBlock)    | §7.7, §18.1, §22             |
| B6 (no sub-day calendar units)            | §3.1                         |
| B7 (maxStatisticalWindows = 3)            | §5, §10.15                   |
| B11 / B12 (per-consumer access table)     | §15                          |
| B13 (StatisticalWindow.count > 0)         | §3.2, §10.11                 |
| C13 (drop CalendarYears / restrict TimeUnit subset) | §3.1, §3.4, §10.18 |
| StatisticalWindow uses tables::TimeUnit   | §3, §3.1, §10.18, §21        |
| StatisticalWindow shared header           | §3, §21                      |
| Shared stattype parser (Option A)         | §22                          |
| typeOfStatisticalProcessing deduction     | §23                          |
| Filename / function naming convention     | §20                          |
| D1 (OptDict_t pass-through, unused)       | §8                           |
| D2 (nested Mars2GribDeductionException)   | §11                          |
| D3 (single composite RESOLVE log)         | §12                          |
| D4 (thread-safety paragraph)              | §14                          |
| D5 (no operator<<)                        | §13                          |
| D6 (full test coverage)                   | §16, §17                     |
| D7 (hard cut for strict alignment)        | §4.4, §18.6                  |
| D10 (step = 0 allowed uniformly)          | §7.5                         |
| R1 (step optional, default 0)             | §7.1, §7.5, §10.1, §17.8     |
| R2 (date/time defaulted from fcyear/fcmonth when both missing) | §7.1, §7.2, §10.1, §17.9 |
| C-list (cosmetic cleanups)                | applied throughout           |

---

## 20. Filename and function-name naming convention

This section is **normative** and applies to every header and function added
or renamed by this specification.

### 20.1 Filenames

Headers under `deductions/` and `deductions/detail/` follow a single rule
based on what the file **primarily exposes**:

| Primary export | Filename initial | Examples                                                           |
|----------------|------------------|--------------------------------------------------------------------|
| A function     | lowercase        | `productTime.h`, `typeOfStatisticalProcessing.h`                   |
| A type         | UpperCamelCase   | `detail/ProductTime.h`, `detail/StatisticalWindow.h`, `detail/StatType.h` |

Public deduction headers under `deductions/` are typically function-primary
(they expose `resolve_<What>_or_throw`) and therefore lowercase. Detail
headers under `deductions/detail/` are typically type-primary (they expose the
data type the public header resolves to) and therefore UpperCamelCase.

A file MUST NOT be renamed once published; this convention applies only to
files added or replaced by this specification.

### 20.2 Function names

Functions in the deductions layer follow the pattern:

```
<verb_lowercase>_<What_UpperCamelCase>_<policy_lowercase>
```

where:

- `<verb_lowercase>` is the operation: `resolve`, `parse`, `make`, `convert`,
  …
- `<What_UpperCamelCase>` is the produced concept or type, in
  UpperCamelCase: `ProductTime`, `StatType`, `TypeOfStatisticalProcessing`,
  `Date`, …
- `<policy_lowercase>` is the failure policy: `or_throw`, `opt`, …

Examples already in the codebase:

```
resolve_ProductTime_or_throw
make_ProductTime_or_throw         (factory; legacy spelling makeProductTime_or_throw is grandfathered)
parse_StatType_or_throw
resolve_TypeOfStatisticalProcessing_or_throw
convert_YYYYMMDD2Date_or_throw
```

The convention is **observed**, not enforced by tooling; reviewers are
responsible.

---

## 21. Shared `StatisticalWindow` type header

### 21.1 Header

```
src/metkit/mars2grib/backend/deductions/detail/StatisticalWindow.h
```

UpperCamelCase initial per §20.1 (type-primary).

### 21.2 Definition

```cpp
#include "metkit/mars2grib/backend/tables/timeUnits.h"

namespace metkit::mars2grib::backend::deductions {

struct StatisticalWindow {
    tables::TimeUnit unit{tables::TimeUnit::Second};
    long             count{0};
};

}  // namespace metkit::mars2grib::backend::deductions
```

The type carries no methods, no `operator<<` (per §13), and no validation;
all validation is performed by the consumers (the parser at parse time, §22;
the factory at assembly time, §10.18).

### 21.3 Consumers

- `deductions/detail/StatType.h` (§22) — produces values of this type.
- `deductions/detail/ProductTime.h` — stores values of this type inside
  `ProductTime::statisticalWindows`.
- `deductions/typeOfStatisticalProcessing.h` (§23) — does NOT consume this
  type (it consumes only the operation field of `ParsedStatTypeBlock`).

### 21.4 Rationale

The type is a shared primitive consumed by both `ProductTime` and the
`stattype` parser. It cannot live inside `detail/ProductTime.h` because that
would force the parser to depend on a deduction's detail header, inverting
the dependency direction. It cannot live inside `detail/StatType.h` because
`ProductTime`'s §9.2 single-loop case constructs `StatisticalWindow`
*without* invoking the parser. A shared header is the only correct
placement.

---

## 22. Shared `stattype` parser

### 22.1 Header

```
src/metkit/mars2grib/backend/deductions/detail/StatType.h
```

UpperCamelCase initial per §20.1 (type-primary; the file exposes the
`ParsedStatTypeBlock` type alongside the parser function).

### 22.2 Type

```cpp
#include "metkit/mars2grib/backend/deductions/detail/StatisticalWindow.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"

namespace metkit::mars2grib::backend::deductions::detail {

struct ParsedStatTypeBlock {
    StatisticalWindow                            timeWindow;
    tables::TypeOfStatisticalProcessing          typeOfStatisticalProcessing;
};

}  // namespace metkit::mars2grib::backend::deductions::detail
```

The `timeWindow` field carries the **period** part of the block (the
stattype-grammar pair `(period_unit, count)`). The
`typeOfStatisticalProcessing` field carries the **operation** part, already
mapped to a value of GRIB Code Table 4.10.

### 22.3 Function

```cpp
namespace metkit::mars2grib::backend::deductions::detail {

std::vector<ParsedStatTypeBlock>
parse_StatType_or_throw(std::string_view value);

}  // namespace metkit::mars2grib::backend::deductions::detail
```

### 22.4 Contract

- **Input**: the raw textual value of the MARS `stattype` keyword. Callers
  MUST NOT invoke the parser when the `stattype` key is absent from the MARS
  dictionary; the absent case is handled at the caller.
- **Output**: a `std::vector<ParsedStatTypeBlock>` of length **1 or 2**, in
  MARS textual order. Index 0 is the leftmost block, which corresponds to
  the **outermost** loop in `ProductTime::statisticalWindows` (matching the
  spec-wide outer→inner convention).
- **Block-count limit**: the parser MUST reject values with zero blocks or
  more than two blocks. The cap of two blocks is the locked grammar limit;
  widening it is an explicit spec amendment.

### 22.5 Allow-lists (parser-level enforcement)

The parser MUST reject blocks that violate the locked `stattype` grammar:

| Field      | Allow-list                                      | Mapped to                                       |
|------------|-------------------------------------------------|-------------------------------------------------|
| period unit| `mo`, `da`                                      | `tables::TimeUnit::Month`, `tables::TimeUnit::Day` |
| operation  | `av`, `mn`, `mx`, `sd`                          | `tables::TypeOfStatisticalProcessing::Average`, `::Minimum`, `::Maximum`, `::StandardDeviation` |
| count      | positive integer                                | `long`                                          |

Period units `Second`, `Hour`, `Year`, `Missing`, etc. are **not** legal in
the `stattype` grammar and the parser MUST reject them at parse time. (The
extended `Second`-inclusive allow-list applies only to the assembled
`statisticalWindows` array inside `ProductTime`, where the innermost window
may originate from `timespan`; see §10.18 for the two-level enforcement
narrative.)

### 22.6 Ordering check

The parser MUST reject `stattype` values whose blocks are not in
outermost-to-innermost period-unit order — i.e., a `da` block MUST NOT
precede a `mo` block. This is hard error §10.17.

### 22.7 Hard errors raised by the parser

| Spec entry | Condition                                                         |
|------------|-------------------------------------------------------------------|
| §10.16     | unknown period or operation token                                 |
| §10.17     | blocks not in outermost-to-innermost order                        |
| §10.18 (a) | period unit outside the narrow `stattype` allow-list `{Day, Month}` |

All other §10 errors are raised by the factory, not the parser.

### 22.8 Rationale

A single shared parser eliminates the drift risk between `productTime` and
`typeOfStatisticalProcessing` (which would otherwise need to size their
output arrays in lock-step from independent parses of the same string).
The parser is shared infrastructure, not a deduction; "self-contained
deduction" means *no dependency on other deductions*, not *no dependency on
shared parsing primitives*.

---

## 23. `typeOfStatisticalProcessing` deduction

### 23.1 Header

```
src/metkit/mars2grib/backend/deductions/typeOfStatisticalProcessing.h
```

Lowercase initial per §20.1 (function-primary). Named after the GRIB key
produced (`typeOfStatisticalProcessing`), consistent with the codebase
convention "filename = concept produced".

The header is **self-contained**: it depends on `detail/StatType.h` (§22)
and `tables/typeOfStatisticalProcessing.h`, but does NOT depend on
`ProductTime`, `detail/ProductTime.h`, or any other deduction.

### 23.2 Prototype

```cpp
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"

namespace metkit::mars2grib::backend::deductions {

template <DictType MarsDict_t, DictType ParDict_t, DictType OptDict_t>
std::vector<tables::TypeOfStatisticalProcessing>
resolve_TypeOfStatisticalProcessing_or_throw(
    tables::TypeOfStatisticalProcessing innerTypeOfStatisticalProcessing,
    const MarsDict_t&                   mars,
    const ParDict_t&                    par,
    const OptDict_t&                    opt);

}  // namespace metkit::mars2grib::backend::deductions
```

`OptDict_t` is passed for signature symmetry with sibling deductions; it is
not currently read.

### 23.3 Implicit invariant — array length and ordering

**Normative.** The output `std::vector<tables::TypeOfStatisticalProcessing>`
has length and order **identical** to `ProductTime::statisticalWindows`
resolved from the same MARS input — outermost loop at index 0, innermost
loop at index `size() - 1`. This invariant is **not** verified at runtime
(this deduction never reads `ProductTime`); callers MUST rely on it
when zipping the two vectors.

The invariant is preserved by the locked `stattype` grammar, the locked
window-assembly cases (§9), and the case-by-case output sizing rules in
§23.5.

### 23.4 Precondition (discharged by the type system)

The function takes a `tables::TypeOfStatisticalProcessing` argument.
A caller cannot construct this argument without having already identified
the product as statistical (i.e., having decoded the inner statistical
operation from `paramId`). Therefore the deduction is **structurally
unreachable** for instant products (§9.1). No runtime check is performed for
this case.

### 23.5 Output sizing per case

The deduction inspects MARS locally (no `ProductTime` dependency) to classify
the input into one of the §9 cases, then sizes its output accordingly:

| MARS state                                              | §9 case | Output size | Output content                                                                  |
|---------------------------------------------------------|---------|------------:|---------------------------------------------------------------------------------|
| `stattype` absent, `timespan` is a Duration             | §9.2    | 1           | `[innerTypeOfStatisticalProcessing]`                                            |
| `stattype` has N blocks (N ∈ {1, 2}), `timespan` is a Duration | §9.3 | N + 1       | `[op(block_0), …, op(block_{N-1}), innerTypeOfStatisticalProcessing]`           |
| `stattype` has exactly 1 block, `timespan = "none"`     | §9.4    | 1           | `[innerTypeOfStatisticalProcessing]` (after equality assertion, §23.6)          |
| `stattype` absent, `timespan` absent                    | §9.1    | unreachable | (precondition violated, see §23.4 — no defensive check)                         |

Where:

- `op(block_i)` is the `typeOfStatisticalProcessing` field of the `i`-th
  `ParsedStatTypeBlock` returned by `parse_StatType_or_throw` (§22).
- The innermost slot is **always** filled by the
  `innerTypeOfStatisticalProcessing` argument — never by a parsed block —
  except for the §9.4 fakeDoubleLoop case where the single slot is filled
  by the argument *after* the equality assertion in §23.6.

### 23.6 fakeDoubleLoop equality assertion (§9.4)

In the §9.4 case, the single parsed `stattype` block carries a redundantly
encoded operation. The deduction MUST assert:

```text
parsedBlocks[0].typeOfStatisticalProcessing == innerTypeOfStatisticalProcessing
```

Disagreement is hard error §10.12. After the assertion succeeds, the single
output slot is filled by `innerTypeOfStatisticalProcessing`. The two values
are equal in this branch by construction; using the argument keeps a single
authoritative source for the inner operation across all cases (§9.2, §9.3,
§9.4).

### 23.7 Outer/inner asymmetry

**Normative.**

- **Outer slots** (indices `0 .. size()-2` in the §9.3 case) are constrained
  to `{Average, Minimum, Maximum, StandardDeviation}` by the parser-level
  op allow-list (§22.5). No other GRIB Code Table 4.10 value can appear in
  an outer slot via the `stattype` path.
- **Innermost slot** (always the last index) is **unrestricted** within
  GRIB Code Table 4.10. It can be any value the caller passes — typically
  `Accumulation`, `Difference`, `Ratio`, `Average`, etc. — derived from
  `paramId` outside this deduction's scope.

Callers consuming the resulting vector against GRIB encoding rules MUST
take this asymmetry into account.

### 23.8 Hard errors

| Spec entry | Condition                                                                                    |
|------------|----------------------------------------------------------------------------------------------|
| §10.12     | §9.4 case with parsed block operation ≠ `innerTypeOfStatisticalProcessing` (§23.6)           |
| §10.16     | propagated from `parse_StatType_or_throw` (§22.7)                                            |
| §10.17     | propagated from `parse_StatType_or_throw` (§22.7)                                            |
| §10.18 (a) | propagated from `parse_StatType_or_throw` (§22.7) — narrow `stattype` unit allow-list        |
| §10.6      | `stattype` present but `timespan` missing (caller-side / consumer-side; this deduction does NOT classify timespan presence beyond what is needed for §23.5; the `timespan`-only checks remain on the `ProductTime` resolver) |

This deduction does **not** raise §10.6, §10.7, or §10.8 directly — those
are `ProductTime`-resolver responsibilities. Misclassification of cases at
this level (e.g., MARS that would fail §10.7 in `ProductTime`) is undefined
behavior here; callers are expected to invoke this deduction only on inputs
that also resolve cleanly through `resolve_ProductTime_or_throw`.

### 23.9 Error contract

Identical to §11. All failures use `Mars2GribDeductionException` with
`Here()` for source location and a nested wrapper:

```cpp
try {
    /* parser call, classification, equality assertion */
}
catch (...) {
    std::throw_with_nested(
        Mars2GribDeductionException(
            "Unable to resolve typeOfStatisticalProcessing", Here()));
}
```

### 23.10 Logging contract

`resolve_TypeOfStatisticalProcessing_or_throw` MUST emit exactly **one** log
line on successful completion, via `MARS2GRIB_LOG_RESOLVE`. Indicative
payload:

```text
`typeOfStatisticalProcessing` resolved from input dictionaries: \
innerTypeOfStatisticalProcessing='...' size='N' \
typesOfStatisticalProcessing=['...','...','...']
```

No log emissions on intermediate sub-steps. No log emissions on failure.

### 23.11 Worked examples

#### 23.11.1 §9.2 — single-loop hourly accumulation

Input:

```text
mars.stattype = (missing)
mars.timespan = "1h"
innerTypeOfStatisticalProcessing = Accumulation
```

Output:

```text
[Accumulation]                       # size 1; matches ProductTime::statisticalWindows = [{Second, 3600}]
```

#### 23.11.2 §9.3 — multi-loop monthly average of daily minimum of hourly accumulation

Input:

```text
mars.stattype = "moav_damn"
mars.timespan = "1h"
innerTypeOfStatisticalProcessing = Accumulation
```

Output:

```text
[Average, Minimum, Accumulation]     # size 3; matches ProductTime::statisticalWindows = [{Month,1}, {Day,1}, {Second,3600}]
```

#### 23.11.3 §9.4 — fakeDoubleLoop monthly average

Input (success):

```text
mars.stattype = "moav"
mars.timespan = "none"
innerTypeOfStatisticalProcessing = Average
```

Output:

```text
[Average]                            # size 1; equality assertion (§23.6) passes
```

Input (failure):

```text
mars.stattype = "moav"
mars.timespan = "none"
innerTypeOfStatisticalProcessing = Maximum
```

Result: hard error §10.12 (parsed `Average` ≠ argument `Maximum`).

