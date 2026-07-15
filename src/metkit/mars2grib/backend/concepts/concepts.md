# Concepts

This document describes the **concept system** used in the *mars2grib* backend and how concepts integrate with the compile-time registry engine.

The goal of the concept layer is to provide a **declarative, type-safe, and fully compile-time description** of all semantic dimensions that participate in GRIB header construction, matching, and encoding.

---

## 1. What is a Concept?

A **concept** represents one *semantic axis* of a GRIB message.

Examples include:

* parameter identity (e.g. temperature, wind)
* vertical level definition
* time semantics
* grid representation
* data packing
* originating centre

Each concept answers the question:

> *Which variant of this semantic dimension applies to the current request?*

Concepts are **independent** of each other and are composed together by the encoder to form a complete GRIB message.

---

## 2. Concept Variants

Each concept defines a **finite set of variants**, usually represented by a scoped enum.

Examples:

* `LevelType::{surface, pressure, model}`
* `RepresentationType::{regular_ll, reduced_gaussian}`
* `PackingType::{simple, jpeg, ccsds}`

Variants are:

* closed and known at compile time
* ordered (the order is semantically significant)
* mapped to GRIB template numbers and encoding rules

The ordered list of variants for a concept defines its **local variant index space**.

---

## 3. New Concept vs New Variant

When changing the concept system, first decide whether the requested behavior is
a new concept or a new variant of an existing concept.

Use a **new concept** when the feature is an independent semantic axis that must
be composable with other concepts. Use a **new variant** when the feature is an
alternative realization inside an existing semantic axis and does not require
independent composability.

This distinction is often a domain decision and is usually not reliably
deducible from the code alone. If a request does not explicitly state whether a
new concept or a new variant is required, ask before implementing.

---

## 4. Level Concept Guardrail

The `level` concept is one of the most constrained concepts in mars2grib.
Although GRIB ultimately represents vertical levels through six low-level
fixed-surface keys:

* `typeOfFirstFixedSurface`
* `scaleFactorOfFirstFixedSurface`
* `scaledValueOfFirstFixedSurface`
* `typeOfSecondFixedSurface`
* `scaleFactorOfSecondFixedSurface`
* `scaledValueOfSecondFixedSurface`

mars2grib must not set these keys directly. Many combinations of these keys are
syntactically possible but semantically meaningless for ECMWF products.

Instead, the encoder must rely on the official level abstraction:

* `typeOfLevel`
* `level`, when required
* `topLevel` / `bottomLevel`, when required
* PV-array data, when required

Each supported `typeOfLevel` corresponds to a `LevelType` variant, apart from a
few virtual type-of-level values kept in mars2grib because they cannot be added
to ecCodes for backward-compatibility reasons. Each variant maps to a prescribed
configuration of the low-level fixed-surface keys.

Do not implement level fixes by injecting `typeOfFirstFixedSurface`,
`scaleFactorOfFirstFixedSurface`, `scaledValueOfFirstFixedSurface`,
`typeOfSecondFixedSurface`, `scaleFactorOfSecondFixedSurface`, or
`scaledValueOfSecondFixedSurface`. If a new level behavior is required, add or
adjust the appropriate `LevelType` variant, matcher mapping, or deduction so the
level remains encoded through `typeOfLevel` and the official level interface.

---

## 5. Concept Descriptor Contract

Each concept is implemented as a **descriptor type** that conforms to the
`RegisterEntryDescriptor` interface.

At minimum, a concept descriptor provides:

* a variant enum type
* a compile-time list of variants (`VariantList`)
* a human-readable concept name
* human-readable variant names
* optional dispatch functions for:

  * matching
  * variant-level encoding
  * phase-level encoding

The descriptor contains **no runtime state** and no virtual functions.

---

## 6. Capabilities

Concepts may expose multiple independent *capabilities*.

A capability is identified by a compile-time `std::size_t` index and allows the
same concept to participate in different registries, such as:

* semantic matching
* encoding
* validation
* diagnostics

Capabilities are selected at registry instantiation time.

This avoids duplicating concept descriptors while still allowing multiple
independent dispatch planes.

---

## 7. The Concept Universe (`AllConcepts`)

All concepts known to the system are aggregated into a single ordered typelist:

```
AllConcepts = TypeList<ConceptA, ConceptB, ConceptC, ...>
```

This list is the **root input** to the compile-time registry engine.

The order of concepts in this list:

* defines the concept identifier (`conceptId`)
* defines the layout of the global variant index space
* determines the ordering of all generated dispatch tables

Changing this order is a **breaking structural change**.

---

## 8. Concept Identifiers

Each concept is assigned a **stable numeric identifier** based on its position
in `AllConcepts`.

```
conceptId = index in AllConcepts
```

Concept identifiers are:

* contiguous
* zero-based
* compile-time constants

They are used as indices into:

* matching callback tables
* concept metadata tables
* diagnostic utilities

---

## 9. Variant Index Spaces

Variants are indexed in two ways:

### Local variant index

The index of a variant *within its concept*.

```
localIndex(variant)
```

### Global variant index

A flattened index across **all concepts and all variants**.

```
globalIndex = conceptOffset + localIndex
```

The global variant index is the primary key used by:

* encoding callback registries
* phase dispatch tables
* encoding plans

---

## 10. Matching Phase

Matching determines **which concepts and variants are active** for a given
input request.

Each concept may optionally provide a matcher function:

```
Fm(MarsDict, OptDict) -> variantId | MISSING
```

The matching phase:

1. iterates over all concepts
2. invokes the corresponding matcher (if present)
3. records the active variant (or absence thereof)

The result is an `ActiveConceptsData` structure.

---

## 11. Encoding Phases

Encoding is divided into **logical stages**, such as:

* allocation
* preset
* override
* runtime

For each active variant, the encoder may execute zero or more callbacks per
(stage, section) pair.

This produces a three-dimensional dispatch space:

```
[variant][stage][section] -> Fn | nullptr
```

All dispatch tables are generated **entirely at compile time**.

---

## 12. Design Principles

The concept system is designed around the following principles:

* **No runtime polymorphism**
* **No dynamic allocation**
* **Deterministic structure**
* **Compile-time validation**
* **Separation of concerns**

Concepts describe *what exists*.
Registries describe *how things are wired*.
Execution code performs *only iteration and invocation*.

---

## 13. Adding a New Concept

To add a new concept:

1. Implement a new descriptor conforming to `RegisterEntryDescriptor`
2. Define its variant enum and `VariantList`
3. Provide matching and/or encoding callbacks as needed
4. Include the descriptor header in `AllConcepts.h`
5. Append the concept to the `AllConcepts` typelist

No registry code needs to be modified.

---

## 14. Summary

Concepts are the **semantic backbone** of the mars2grib backend.

They provide:

* a declarative description of the domain
* compile-time structure and guarantees
* a clean separation between semantics and execution

All higher-level mechanisms — matching, layout resolution, encoding — are
built mechanically on top of the concept system.
