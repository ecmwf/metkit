# ProductTimeSpec Resolver Companion Specification

> Status: companion specification.
>
> This document is a focused companion to `productTimeSpecV3_final.md`. It does
> not replace the monolithic specification. Instead, it reorganizes the same
> ProductTimeSpec model from the perspective of classification, construction,
> canonicalization, and final validation so that the public resolver entry point
> and the `detail/resolver/` headers can be documented and maintained
> independently.

---

## 1. Scope

This companion document covers the ProductTimeSpec frontend resolver after a
complete normalized `ProductTimeSpecInput` snapshot already exists.

It therefore focuses on:

- the classifier-first resolver pipeline;
- the three classification axes;
- the anchor, shape, and increment resolvers;
- cross-classification consistency checks;
- semantic construction artifacts;
- canonicalization of the final immutable `ProductTimeSpec`;
- final whole-object invariant checks;
- resolver-layer diagnostic failures.

It does not cover the detailed extraction and normalization boundary itself;
that belongs to `ProductTimeSpecInput_spec.md`.

---

## 2. Relationship To The Full Specification

This document reorganizes material already present in the full specification,
mainly from:

- Section 3.2: Resolver Pipeline;
- Section 3.3: Classification Axes;
- Sections 4.11 through 4.18: resolver artifacts and exception model;
- Sections 5.4 through 5.16: canonical semantics and invariants;
- Section 6: Classifier And Construction Architecture;
- Section 8: post-input error handling.

When this companion and the full specification overlap, the full specification
remains the canonical source of truth.

---

## 3. Resolver Responsibilities

After extraction has produced `ProductTimeSpecInput`, the frontend resolver owns:

- classification of time-anchor, shape, and increment semantics;
- representation-policy checks that depend on normalized input;
- construction of semantic artifacts:
  - `ProductTimeSpecAnchor`
  - `ProductTimeSpecShape`
  - `ProductTimeSpecIncrement`
- validation of relationships across independently valid classifications;
- canonicalization of the final immutable `ProductTimeSpec`;
- final whole-object invariant checks.

Backends do not reinterpret raw MARS keys. They consume only the resolved final
ProductTimeSpec object.

---

## 4. Resolver Pipeline

The architecture remains classifier-first.

Conceptually:

```text
ProductTimeSpecInput
    -> ProductTimeSpecClassification
    -> ProductTimeSpecAnchor
    -> ProductTimeSpecShape
    -> ProductTimeSpecIncrement
    -> ProductTimeSpec canonical IR
```

The practical pipeline is:

1. classify anchor;
2. classify shape;
3. classify increment;
4. validate cross-classification consistency;
5. construct anchor;
6. construct shape;
7. construct increment;
8. canonicalize the final ProductTimeSpec;
9. validate final whole-object invariants.

---

## 5. Classification Axes

### 5.1 Time anchor classification

`TimeAnchorKind` determines how:

- `labelDateTime`;
- `initialConditionsDateTime`;
- `referenceDateTime`

are sourced.

It depends only on normalized direct-anchor source presence and rejects local
anchor cross-key states such as:

- no direct anchor source among `date`, `hdate`, and complete `year`/`month`;
- `time` without `date`.

### 5.2 Shape classification

`ProductTimeSpecShapeKind` determines whether the product is:

- `Instant`;
- `StandardSingleLoop`;
- `MultiLoop`;
- `FakeDoubleLoopSingleLoop`;
- `FromStartSingleLoop`.

It depends on:

- normalized `timespan` representation;
- parsed `stattype` structure;
- fakeDoubleLoop representation policy;
- from-start restrictions;
- zero-length from-start policy.

### 5.3 Increment classification

`TimeIncrementKind` determines whether the product uses:

- `NoIncrement`;
- `ExplicitIncrement`;
- `DefaultedIncrement`;
- `AifsPureMissingIncrement`.

It depends on:

- explicit increment presence;
- frontend real-window counting;
- shape kind;
- option-side defaulting policy;
- AIFS single-window missing-increment policy.

---

## 6. Anchor Resolver

### 6.1 Classification

The anchor classifier determines `TimeAnchorKind` from the direct special-anchor
sources:

| Direct `hdate` | Direct `year` / `month` | Result |
|----------------|-------------------------|--------|
| absent | absent | `LabelOnly` |
| present | absent | `Hindcast` |
| absent | present | `ForecastAnchor` |
| present | present | `HindcastForecastAnchor` |

It does not perform final ordering checks, because ordering can only be checked
after inheritance during anchor construction.

### 6.2 Construction

Anchor construction materializes:

- `labelDateTime`;
- `initialConditionsDateTime`;
- `referenceDateTime`.

It applies the direct-source and inheritance rules and then checks:

```text
labelDateTime <= initialConditionsDateTime <= referenceDateTime
```

The resulting artifact is `ProductTimeSpecAnchor`.

---

## 7. Shape Resolver

### 7.1 Shape classification

The shape classifier owns:

- `timespan` / `stattype` structural compatibility;
- fakeDoubleLoop representation policy;
- from-start rejection of any `stattype` block;
- zero-length from-start rejection when disabled;
- analysis-step consistency around missing or non-zero step.

### 7.2 Shape construction

Shape construction materializes:

- absolute `windowStartDateTime`;
- absolute `windowEndDateTime`;
- optional `innerTimeRange` where structurally applicable;
- parsed structural outer windows from `stattype`;
- the `zeroLengthFromStartWindowByDesign` evidence flag.

It also owns outermost calendar alignment checks for day- and month-based
outer windows.

The resulting artifact is `ProductTimeSpecShape`.

---

## 8. Increment Resolver

### 8.1 Increment classification

The increment classifier owns:

- frontend real-window counting through the helper concept used before final IR
  exists;
- redundant increment handling where the final semantic increment is missing;
- explicit increment acceptance;
- eligible defaulted increment policy;
- explicit rejection of defaulting for non-`ml` `FromStartSingleLoop`;
- AIFS-pure missing-increment policy;
- missing increment rejection.

### 8.2 Increment construction

Increment construction materializes one `ProductTimeSpecIncrement` artifact.

By semantic kind:

- `NoIncrement` -> zero-second missing sentinel;
- `ExplicitIncrement` -> positive materialized elapsed duration plus non-missing
  `typeOfTimeIncrement`;
- `DefaultedIncrement` -> positive materialized defaulted duration plus
  non-missing `typeOfTimeIncrement`;
- `AifsPureMissingIncrement` -> zero-second missing sentinel.

The resulting artifact is `ProductTimeSpecIncrement`.

---

## 9. Cross-Classification Consistency

The cross-classification stage validates relationships between independently
valid local classifications.

Current rules include:

- `NoIncrement` only with `Instant`;
- `AifsPureMissingIncrement` only with exactly one real statistical window;
- `DefaultedIncrement` not with `class == "ml"`;
- `DefaultedIncrement` not with `FromStartSingleLoop`;
- instant products require innermost processing `Missing`;
- non-instant supported statistical products require innermost processing
  non-`Missing`;
- from-start products require innermost processing compatible with supported
  from-start semantics, currently `Accumulation`;
- fakeDoubleLoop caller-supplied innermost processing must equal the non-missing
  processing parsed from its single `stattype` block.

This stage must succeed before any semantic construction artifact is accepted as
part of the resolver pipeline.

---

## 10. Construction Artifacts

The resolver constructs three intermediate semantic artifacts before the final
canonical IR exists.

1. `ProductTimeSpecAnchor`

   Stores the three resolved anchor datetimes and the anchor kind.

2. `ProductTimeSpecShape`

   Stores absolute support placement, structural time-range sources, and the
   shape kind.

3. `ProductTimeSpecIncrement`

   Stores the resolved materialized increment and its increment kind.

These artifacts are not backend inputs. They exist to keep the frontend clear,
classified, and branch-specific.

---

## 11. Canonicalization

Canonicalization consumes:

- `ProductTimeSpecInput`;
- `ProductTimeSpecClassification`;
- `ProductTimeSpecAnchor`;
- `ProductTimeSpecShape`;
- `ProductTimeSpecIncrement`.

It must:

1. construct the canonical `ProductTimeWindows` sequence;
2. materialize per-window `typeOfStatisticalProcessing`;
3. materialize per-window `typeOfTimeIncrement`;
4. materialize per-window `timeRange` and `timeIncrement`;
5. validate the increment-within-window rule for every real canonical window;
6. store the option snapshot;
7. store `kind` and `incrementKind`;
8. construct the complete immutable `ProductTimeSpec` candidate.

This stage is also where the backend-facing temporal meaning becomes fixed.

---

## 12. Final Consistency

After canonicalization, the final whole-object invariant stage validates the
complete immutable ProductTimeSpec candidate.

Representative invariants include:

- anchor ordering;
- support interval ordering;
- shape-specific canonical range cardinality;
- absolute support agreement with the outermost canonical range;
- from-start support beginning at `referenceDateTime`;
- AIFS missing-increment sentinel invariants;
- agreement between frontend and final-IR real-window counts.

Only after these final checks succeed is the ProductTimeSpec candidate returned
as the final canonical IR.

---

## 13. Resolver Error Handling

After a complete `ProductTimeSpecInput` snapshot exists, internal resolver
failures use `Mars2GribProductTimeSpecException`.

That exception carries:

- stage;
- reason;
- normalized input JSON;
- classification JSON when available;
- construction-artifact JSON when available;
- final ProductTimeSpec JSON when available.

The resolver therefore preserves enough context for stage-specific diagnostics
without forcing downstream users to reconstruct semantic state from raw source
dictionaries.

---

## 14. Backend-Facing Consequences

Backends consume only the final canonical `ProductTimeSpec`.

Point-in-time lowering uses:

- the resolved anchor;
- `windowEndDateTime`.

Statistics lowering uses:

- `referenceDateTime`;
- `windowStartDateTime`;
- `windowEndDateTime`;
- `numberOfTimeRanges()`;
- the ordered canonical windows.

No backend should reinterpret raw MARS keys once resolver construction has
completed.

---

## 15. Mapping To Code

This companion specification corresponds primarily to:

- `ProductTimeSpecResolver.h`;
- `detail/resolver/ProductTimeSpecResolverCommon.h`;
- `detail/resolver/ProductTimeSpecAnchorResolver.h`;
- `detail/resolver/ProductTimeSpecShapeResolver.h`;
- `detail/resolver/ProductTimeSpecIncrementResolver.h`;
- `detail/resolver/ProductTimeSpecCrossClassification.h`;
- `detail/resolver/ProductTimeSpecCanonicalization.h`;
- `detail/resolver/ProductTimeSpecFinalConsistency.h`.
