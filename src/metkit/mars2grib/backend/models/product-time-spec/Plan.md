<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# ProductTimeSpec — Check-Wiring & Matcher-Diagnostics Plan

> **Cold-start note.** This file is written to be self-contained. A fresh session
> should be able to resume from this file alone. Read this whole document first,
> then the "Files to read before editing" list, then begin at
> **Execution Order**. Do NOT start editing before reading the referenced files;
> line numbers below were accurate at authoring time but MUST be re-confirmed
> since the code may have shifted.

---

## 0. Session context (how we got here)

This subsystem lives under:
`src/metkit/mars2grib/backend/models/product-time-spec/`

Two pieces of prior work are already **complete and committed to the working
tree** (do not redo):

1. **`enableOptions.h` migration (DONE).** All boolean model-policy helpers
   (`checksEnabled`, `skipSection3`, `bitsPerValueCompressionEnabled`,
   `normalizeMarsEnabled`, `normalizeMiscEnabled`, `fixMarsGridEnabled`, plus
   dead `overrideEnabled`) were replaced by direct
   `get_or_throw<bool>(opt, "<key>")` trait calls. The header
   `src/metkit/mars2grib/utils/enableOptions.h` was DELETED and no longer exists.
   Rule going forward: use strict `get_or_throw<bool>(opt, "<key>")`; never
   `.value_or(false)`. If a default is ever needed, add it inside the traits.

2. **Read-only review of `product-time-spec` (DONE).** Findings are captured in
   section 2 below. This Plan.md is the output of that review.

**Hard constraints for the executing agent:**
- The USER builds, runs, debugs, and commits. Do NOT build, run, or commit.
- Make minimal, surgical changes; touch only what each phase specifies.
- Every nontrivial function body is wrapped in `try { ... } catch (...) {
  std::throw_with_nested(Mars2GribModelException(msg, input.to_json(), Here())); }`.
  Input-free helpers use the location-only exception constructor.
- Diagnostic `to_json()` serializers are `noexcept` with a stable fallback
  string. Preserve this.
- Companion docs in the same directory: `Requirements.md` (authoritative task
  tree), `Status.md` (live status — currently STALE, see Phase 0),
  `Constraints.md` (hard constraints).

---

## 1. Subsystem architecture (so a fresh session need not re-read everything)

**Top-level model:** `ProductTimeSpec.h` exposes two immutable classes:
- `ProductTimeAnchorSpec` — anchor-only reduced model.
- `ProductTimeSpec` — full model.

Both use **staged immutable construction**: a public templated ctor takes
`(mars, par, opt)` dicts, builds a normalized `ProductTimeSpecInput`, then a
private static `build_*Components_or_throw(input)` runs the pipeline and returns
a component bundle consumed by the final private ctor that initializes `const`
members.

**Full pipeline** — `ProductTimeSpec.h`,
`build_ProductTimeSpecComponents_or_throw` (≈:366-405):
1. `anchor::classify_Anchor_or_throw(input)` -> `anchorType`
2. `domain::classify_Domain_or_throw(input)` -> `domainType`
3. `shape::classify_Shape_or_throw(input)` -> `shapeType`
4. assemble `ProductTimeSpecClassification` bundle
5. `anchor::build_Anchor_or_throw(anchorType, input, classification)` -> `anchor`
6. `shape::build_ShapeOuterTimeRange_or_throw(shapeType, input, classification)`
   -> `outerTimeRange`
7. `domain::build_Domain_or_throw(domainType, input, classification, anchor,
   outerTimeRange)` -> `domain`
8. `shape::build_ShapeWindows_or_throw(shapeType, input, classification, anchor,
   outerTimeRange, domain)` -> `rawWindows`
9. `detail::normalizeShape_or_throw(input, domain, rawWindows)` ->
   `normalisedWindows`   (already wired, ≈:387-388)
10. bundle + freeze.

**Registries** (`anchors/AnchorRegistry.h`, `domains/DomainRegistry.h`,
`shapes/ShapeRegistry.h`): each defines an immutable `std::array` of case rows,
one per enum value, each row = `{classification, name, matcher, builder(s),
checker}`, with `static_assert`s locking array index to enum value. Each exposes:
- `classify_*_or_throw` — evaluates EVERY matcher, requires EXACTLY one `true`;
  0 or >1 is a hard `Mars2GribModelException`. Non-prioritized, order never
  resolves overlap.
- `build_*_or_throw` — index-dispatches the builder.
- `check_*_or_throw` — index-dispatches the checker. **Currently defined but
  NEVER CALLED anywhere.** This is the core gap Workstream A fixes.

**Registry dispatcher signatures (verified — do not guess):**
- `anchor::check_Anchor_or_throw(ProductTimeSpecAnchorKind, const
  ProductTimeSpecInput&, const anchor::ProductTimeSpecAnchor&) -> bool`
  (`AnchorRegistry.h:198`)
- `domain::check_Domain_or_throw(ProductTimeSpecDomainKind, const
  ProductTimeSpecInput&, const anchor::ProductTimeSpecAnchor&, const
  domain::ProductTimeSpecDomain&) -> bool`  (`DomainRegistry.h:239`)
- `shape::check_Shape_or_throw(ProductTimeSpecShapeKind, const
  ProductTimeSpecInput&, const ProductTimeSpecClassification&, const
  anchor::ProductTimeSpecAnchor&, const ProductTimeSpecOuterTimeRange&, const
  domain::ProductTimeSpecDomain&, const ProductTimeSpecShape&) -> bool`
  (`ShapeRegistry.h:321`)

All three checkers return `true` on success and `throw` on real failure. When
wiring, still guard the `bool`: a non-`true` return is a hard error.

**Matcher function-pointer types (current):**
- `AnchorMatcher = bool (*)(const ProductTimeSpecInput&)` (`AnchorRegistry.h:65`)
- `DomainMatcher = bool (*)(const ProductTimeSpecInput&)` (`DomainRegistry.h:66`)
- `ShapeMatcher  = bool (*)(const ProductTimeSpecInput&)` (`ShapeRegistry.h:73`)

**Enum counts (for scope):** 3 anchor cases, 5 domain cases, 14 shape cases =
**22 matchers total**.
- Anchors: ForecastAnalysis, Hindcast, SeasonalClimate.
- Domains: ForecastDomain, FromStartForecastDomain, SeasonalForecastDomain,
  AnalysisDomain, SynopticAnalysisDomain.
- Shapes: Instant, IFSStandardSingleLoop, IFSFakeDoubleLoopSingleLoop,
  IFSFromStartSingleLoopAtZero, IFSFromStartSingleLoopPositive,
  IFSSynopticSingleLoop, AIFSStandardSingleLoop, AIFSFakeDoubleLoopSingleLoop,
  AIFSFromStartSingleLoopAtZero, AIFSFromStartSingleLoopPositive,
  SeasonalSingleLoop, SeasonalMultiloop, IFSStandardMultiLoop,
  IFSFakeSingleLoopDoubleLoop.

**Per-case impl files** (matcher + builders + checker live together per case):
- `anchors/impl/{ForecastAnalysis,Hindcast,SeasonalClimate}.h`
- `domains/impl/{AnalysisDomain,ForecastDomain,FromStartForecastDomain,
  SeasonalForecastDomain,SynopticAnalysisDomain}.h`
- `shapes/impl/*.h` (14 files, names match the shape kinds above).

**Reference impl already read (canonical example):** `shapes/impl/Instant.h`
contains `match_Instant_Shape`, `build_Instant_ShapeOuterTimeRange`,
`build_Instant_ShapeWindows`, `check_Instant_Shape`. Use it as the pattern
template for both workstreams.

**Normalization:** `detail/ShapeNormalization.h`,
`normalizeShape_or_throw(input, domain, rawShape)`:
- `timeRange` -> whole hours; month-based ranges converted via the EXACT placed
  interval from the resolved domain (domain not mutated; synoptic uses
  `defaultMarsTime()` start).
- `timeIncrement` -> whole seconds or missing.
- Whitelists: sub-monthly hours `{1,3,6,12,18,24,48,72,120,168,240,360}`
  (:194-195); monthly hours `{672,696,720,744}` (:196).
- **Contains an outermost-span/window-consistency check at :235-258** (real
  domain span in hours must equal outermost normalized window). Phase 2 MOVES
  this into the cross-check stage.

**Shared helpers:**
- `domains/DomainUtils.h`: `resolvedForecastStep`, `resolvedSeasonalForecastLead`,
  `timespanDuration`, `resolveOuterDomainRange` (:163),
  `resolveSeasonalForecastOuterDomainRange` (:222) [near-duplicate of the former,
  differs only in from-start branch], and
  `offsetHoursFromReference(referenceDateTime, targetDateTime) -> long` (:274)
  (signed whole-hour offset; throws on non-whole-hour or overflow).
- `shapes/ShapeUtils.h`: `timespanIsNone(input)`,
  `timespanIsMissingAndAllowed(input, allow)`.
- `anchors/AnchorDataTypes.h`: `ProductTimeSpecAnchor` artifact (`labelDateTime`,
  `initialConditionsDateTime`, `referenceDateTime`, `anchorType`) with invariant
  `label <= initialConditions <= reference`; `checkedAnchor(...)` (input-free,
  location-only exceptions); `productTimeSpecAnchorJson(...)` (`noexcept`).

**Domain artifact** (`domains/DomainDataTypes.h`) carries (per agreed decisions
in `Status.md:66-70`): absolute `domainStartDateTime`/`domainEndDateTime`,
`bool isSynoptic`, `long startOffsetHoursFromReference`,
`long endOffsetHoursFromReference`.

**The seasonal discriminator predicate is copy-pasted** in at least:
`DomainUtils.h:83-86`, `Instant.h:68-71` (matcher), `Instant.h:238-241`
(checker), and other shape matchers/checkers. Pattern:
```cpp
const bool hasSeasonalClassStream =
    (input.marsClass == "od" || input.marsClass == "rd" || input.marsClass == "c3") &&
    (input.marsStream == "sfmd" || input.marsStream == "shmd");
const bool hasSeasonalLeadSemantics = !input.step.has_value() && input.marsFcmonth.has_value();
// seasonal == hasSeasonalClassStream && hasSeasonalLeadSemantics
```

---

## 2. Review findings (the "why" behind this plan)

- **(A) Dead validation layer.** All `check_*` callbacks + `check_*_or_throw`
  dispatchers exist and are thorough (e.g. `check_Instant_Shape` is ~90 lines),
  but NONE are invoked in the pipeline. -> Workstream A, Phase 1.
- **(B) `Status.md` is stale.** It claims normalization is "not wired yet"
  (`:32`, `:217`, `:284`), but `ProductTimeSpec.h:387-388` already calls
  `normalizeShape_or_throw`. -> Phase 0.
- **(C) Raw-vs-normalized check ordering only half-present.** Docstrings say
  checkers validate the RAW shape and that normalization runs "after raw checks
  succeed", but no check currently runs before OR after normalization.
  -> Phases 1 + 3.
- **(D) `DomainUtils.h` outer-range duplication** (`resolveOuterDomainRange` vs
  `resolveSeasonalForecastOuterDomainRange`). Intentional per its doc comment;
  left OUT OF SCOPE (drift risk noted).
- **(E) Seasonal discriminator copy-paste.** -> Phase 4 (optional but
  recommended) extracts a single shared predicate.

---

## 3. Decisions locked with the user (do not re-litigate)

1. **Checker wiring = two phases:** wire per-case raw checkers right after each
   raw build (Phase 1), THEN add a separate post-normalization cross-check stage
   (Phase 3). (Chosen over a single post-normalization pass.)
2. **Span check moves** out of `normalizeShape_or_throw` into the new
   cross-check stage (Phase 2). Normalization only transforms; validation lives
   in the check stage.
3. **Only matchers are templated** on the mode enum (Workstream B). Builders and
   checkers keep their signatures.
4. **Explain-mode uses a mode-templated `MatchAccumulator`** (single source of
   truth for condition names; bool result and JSON derive from the same
   `record()` calls). Chosen over hand-written `if constexpr` JSON.
5. Plan file location: this file, `product-time-spec/Plan.md`.
6. Strict `get_or_throw<bool>` everywhere for options (from prior workstream).

---

## 4. Execution order (do phases top-to-bottom)

Recommended sequencing so each step is independently verifiable by the user:
**Phase 0 -> 1 -> 2 -> 3**, then **Phase 5 (B)**, then **Phase 4** last
(broad touch, easiest to review in isolation). Phase 4 is optional; confirm with
user if time-constrained. Phases 2 and 3 must land together (Phase 2 removes a
check that Phase 3 re-homes) so no validation gap exists between commits.

---

## Workstream A — Checks & Cross-Checks

### Phase 0 — Reconcile `Status.md` (doc-only)
- `:32` "normalization helper added in code but not wired yet" -> "wired into the
  top-level pipeline".
- Normalization Tracker `:215-217`: "top-level wiring not implemented yet" ->
  "wired in `build_ProductTimeSpecComponents_or_throw`".
- `Next Session Entry Point` (`:278-286`): drop "wiring normalization" (done);
  replace with "wire per-case raw checkers + add cross-check stage".
- Leave Cross-Check Tracker (`:233-245`) "not implemented" until Phase 3 lands,
  then flip Future Raw Check entries (`:173-204`) from `added` -> `wired` and
  Cross-Check -> `implemented`.

### Phase 1 — Wire per-case raw checkers
File: `ProductTimeSpec.h`, `build_ProductTimeSpecComponents_or_throw`
(≈:366-405). No changes to checker code (signatures already match §1). Insert,
inside the existing `try`, each guarded so a non-`true` return throws
`Mars2GribModelException(..., input.to_json(), Here())`:
1. After `build_Anchor_or_throw` (≈:379):
   `anchor::check_Anchor_or_throw(anchorType, input, anchor)`
2. After `build_Domain_or_throw` (≈:382-383):
   `domain::check_Domain_or_throw(domainType, input, anchor, domain)`
3. After `build_ShapeWindows_or_throw` (raw, ≈:384-385), BEFORE
   `normalizeShape_or_throw`:
   `shape::check_Shape_or_throw(shapeType, input, classification, anchor,
   outerTimeRange, domain, rawWindows)`
Also mirror step 1 into
`ProductTimeAnchorSpec::build_ProductTimeAnchorSpecComponents_or_throw`
(≈:199-220), after its `build_Anchor_or_throw` (≈:206).
(Includes for the registries are already present in `ProductTimeSpec.h:51-55`.)

### Phase 2 — Move span check out of normalization
File: `detail/ShapeNormalization.h`. Remove the outermost-span validation block
(≈:235-258): the `realDomainStart`, `realDomainSpan`, `realDomainSpanInHours`,
`outermostWindow` locals and their throws. Keep ALL pure unit transforms
(timeRange->hours incl. month placement, timeIncrement->seconds/missing,
whitelist checks at :194-228). The month-placement logic (:134-156) is
independent and STAYS. Re-verify no other code in the function references the
removed locals before deleting.

### Phase 3 — Post-normalization cross-check stage
New file: `detail/ProductTimeSpecCrossChecks.h`. Conventions: license header
(copy from any sibling), `@file`/`@brief`/`@ingroup
mars2grib_product_time_spec_detail` doc block, namespace
`metkit::mars2grib::backend::models::product_time_spec::detail`, full-body
try/catch + nested `Mars2GribModelException(..., input.to_json(), Here())`.

Signature:
```cpp
inline void crossCheck_ProductTimeSpec_or_throw(
    const ProductTimeSpecInput& input,
    const anchor::ProductTimeSpecAnchor& anchor,
    const domain::ProductTimeSpecDomain& domain,
    const shape::ProductTimeSpecShape& normalizedWindows);
```
Checks (each a named `const bool ...` + explicit throw on violation; source:
`Status.md:235-241`):
1. **Domain span vs outermost window** (relocated from Phase 2). Recompute real
   domain span in whole hours: `start = domain.isSynoptic ?
   makeDateTime(domain.domainStartDateTime.date(), defaultMarsTime()) :
   domain.domainStartDateTime;` `span = durationBetween(start,
   domain.domainEndDateTime);` assert `span.unit == Second`, `span.length %
   3600 == 0`, and `normalizedWindows.values.front().timeRange == {span/3600,
   Hour}`.
2. **Domain offsets vs reference:** assert
   `domain.startOffsetHoursFromReference == offsetHoursFromReference(
   anchor.referenceDateTime, domain.domainStartDateTime)` and same for end.
   (`offsetHoursFromReference` in `DomainUtils.h:274`.)
3. **Synoptic consistency:** `domain.isSynoptic == input.isSynoptic`.
4. **Seasonal consistency:** `(domain kind == SeasonalForecastDomain) ==
   isSeasonalProduct(input)` (use the Phase 4 predicate; if Phase 4 is skipped,
   inline the predicate here).
5. **Normalized-hour whitelist re-assertion** on final artifacts
   (defense-in-depth mirroring `ShapeNormalization.h:194-196`).

Wiring: in `build_ProductTimeSpecComponents_or_throw`, call
`detail::crossCheck_ProductTimeSpec_or_throw(input, anchor, domain,
normalisedWindows);` immediately after `normalizeShape_or_throw` (≈:388), before
assembling `result`. Add `#include ".../detail/ProductTimeSpecCrossChecks.h"` to
`ProductTimeSpec.h`.

### Phase 4 — De-duplicate seasonal discriminator (recommended, do LAST)
Add a single predicate — location options: a free
`inline bool isSeasonalProduct(const ProductTimeSpecInput&)` in a shared detail
header, or a member on `ProductTimeSpecInput`. Capture:
`hasSeasonalClassStream && hasSeasonalLeadSemantics` (exact expression in §1).
Replace all copies: `DomainUtils.h:83-86`, `Instant.h:68-71`, `Instant.h:238-241`,
and grep the other shape/domain impl files for the same two-line pattern and
replace each. Behavior must be identical — pure extraction.

---

## Workstream B — Mode-Templated Matchers with JSON Diagnostics

### Intent
Each matcher runs in two modes: **Evaluate** (returns `bool`, production) and
**Explain** (returns a JSON string of every named condition + value, debug, NO
short-circuit). On classification failure (0 or >1 matches), the
`classify_*_or_throw` failure path calls each case's Explain matcher and attaches
the aggregated JSON to the thrown exception, so the stack carries exactly which
conditions were true/false per case.

### Phase 5.0 — Shared mode primitives
New file: `detail/MatcherMode.h`. License header + doc block + namespace
`...::product_time_spec::detail`.
```cpp
enum class MatchMode { Evaluate, Explain };

template <MatchMode Mode>
using MatchResult = std::conditional_t<Mode == MatchMode::Explain, std::string, bool>;

template <MatchMode Mode>
class MatchAccumulator {
public:
    // Evaluate: ANDs value into running result.
    // Explain:  appends {"<name>": <bool>} to an ordered JSON object.
    void record(std::string_view name, bool value);
    // Evaluate: returns the conjunction (empty == true; every matcher records
    //           >=1 condition, so document this edge case).
    // Explain:  returns a complete JSON object string, e.g.
    //           {"cond1":true,"cond2":false,...,"__match__":false}
    MatchResult<Mode> result() const;
private:
    // Evaluate storage: bool accumulator (init true).
    // Explain storage: std::ostringstream + first-flag + running bool for
    //                  the trailing "__match__" summary field.
};
```
Notes: `record()` must be called for EVERY condition unconditionally (no early
return) so Explain reports all of them; Evaluate still gets the correct AND
because all bools are computed before recording (matchers already compute all
named bools eagerly). Include `<sstream>`, `<string>`, `<string_view>`,
`<type_traits>`. Reuse existing JSON-quote helper if one exists in
`detail/ProductTimeSpecJsonUtils.h` (check `jsonQuote_modelInput`) for
consistent escaping.

### Phase 5.1 — Convert all 22 matcher signatures
For each matcher in the 3+5+14 impl files, change
`inline bool match_X(const ProductTimeSpecInput& input)` to:
```cpp
template <MatchMode Mode>
inline MatchResult<Mode> match_X(const ProductTimeSpecInput& input) {
    using ...detail::MatchAccumulator; using ...detail::MatchMode;
    try {
        MatchAccumulator<Mode> acc;
        // compute the SAME named bools as today
        acc.record("condNameA", condA);
        acc.record("condNameB", condB);
        // ... one record() per existing named bool ...
        return acc.result();
    }
    catch (...) {
        std::throw_with_nested(Mars2GribModelException(
            "Failed to execute `match_X`", input.to_json(), Here()));
    }
}
```
Reference conversion — `match_Instant_Shape` (currently `Instant.h:61-86`)
records: `isNotSeasonal`, `hasAcceptedTimespanRepresentation`,
`hasNoStattypeBlocks`, `hasMissingStatisticalProcessing`. Keep the exact bool
computations; only replace the final `return A && B && C && D;` with four
`acc.record(...)` + `return acc.result();`.
Do NOT template builders or checkers.

### Phase 5.2 — Registry rows carry both instantiations
In each registry (`AnchorRegistry.h`, `DomainRegistry.h`, `ShapeRegistry.h`):
- Add an Explain pointer type beside the existing matcher type, e.g.
  `using ShapeMatcher = bool (*)(const ProductTimeSpecInput&);`
  `using ShapeMatcherExplain = std::string (*)(const ProductTimeSpecInput&);`
- Add a `matcherExplain` field to the case struct (`AnchorCase`/`DomainCase`/
  `ShapeCase`) right after `matcher`.
- In each row, set `matcher = &match_X<MatchMode::Evaluate>` and
  `matcherExplain = &match_X<MatchMode::Explain>`. (This changes the existing
  `&match_X` entries — every row in `anchorCases`/`domainCases`/`shapeCases`.)
- The `static_assert`s on classification index are unaffected.

### Phase 5.3 — Emit diagnostics on classification failure
In each `classify_*_or_throw`, the failure branch currently builds a
`name=true/false` summary (see `ShapeRegistry.h:203-218` and
`AnchorRegistry.h:139-143`, `DomainRegistry.h:160-164`). Replace/augment: when
`numberOfMatches != 1`, iterate all cases calling `.matcherExplain(input)` and
assemble a JSON array
`[{"case":"<name>", "conditions":<explainJson>}, ...]`, and include it in the
thrown `Mars2GribModelException` message (alongside `input.to_json()`). Keep
this inside the existing failure path so the Explain cost only occurs on error.

### Design guarantees
- Explain path is failure-only -> zero production success-path cost.
- One function body per matcher -> Explain conditions can never drift from
  Evaluate conditions.
- Builders/checkers untouched -> blast radius limited to matchers + registries.

---

## 5. Files to read before editing (fresh session checklist)

Must-read (re-confirm line numbers):
- `ProductTimeSpec.h` (pipeline; Phases 1, 3 wiring).
- `anchors/AnchorRegistry.h`, `domains/DomainRegistry.h`,
  `shapes/ShapeRegistry.h` (Phases 1, 5.2, 5.3).
- `detail/ShapeNormalization.h` (Phase 2).
- `domains/DomainUtils.h` (`offsetHoursFromReference` for Phase 3; dedup source
  for Phase 4).
- `shapes/impl/Instant.h` (reference pattern for Phases 4, 5.1).
- `detail/ProductTimeSpecJsonUtils.h` (JSON-quote helper for Phase 5.0).
- `Status.md` (Phase 0 edits).
Skim before Phase 5.1 (22 matchers): all `anchors/impl/*.h`,
`domains/impl/*.h`, `shapes/impl/*.h`.

## 6. Verification (USER runs; agent must NOT)
- Build the metkit target.
- Existing ProductTimeSpec tests pass; the relocated span check still fires (now
  from the cross-check stage); newly-wired per-case checkers do not reject
  previously-valid inputs.
- Construct a deliberate 0-match and a >1-match input; confirm the thrown
  exception carries the per-condition Explain JSON for every case.

## 7. Out of scope (deferred per `Status.md`)
- `DomainUtils.h` outer-range dedup (finding D).
- Error-boundary audit; `TemporalArithmetic` hardening (`Status.md` phases 4-5).
- Detailed paranoia/check-level option structure (`Status.md` Deferred).

## 8. Progress log (update as phases land)
- [ ] Phase 0 — Status.md reconciled
- [ ] Phase 1 — per-case raw checkers wired (full + anchor-only paths)
- [ ] Phase 2 — span check removed from normalization
- [ ] Phase 3 — ProductTimeSpecCrossChecks.h added + wired
- [ ] Phase 5.0 — MatcherMode.h (enum + MatchAccumulator)
- [ ] Phase 5.1 — 22 matchers templated
- [ ] Phase 5.2 — registries carry Explain pointers
- [ ] Phase 5.3 — classify failure emits Explain JSON
- [ ] Phase 4 — seasonal predicate de-duplicated (optional, last)
