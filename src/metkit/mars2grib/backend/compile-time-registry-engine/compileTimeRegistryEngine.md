# Compile-Time Registry Engine

## Overview

The **compile-time registry engine** is the backbone of the mars2grib dispatch and encoding infrastructure. Its purpose is to transform *static knowledge about concepts, variants, stages, sections, and capabilities* into **fully materialized, constexpr dispatch tables** that can be used directly at runtime with:

* no dynamic allocation,
* no virtual dispatch,
* no string-based lookup in hot paths,
* no runtime branching.

In short:

> **All decisions are made at compile time. Runtime code only indexes arrays and calls function pointers.**

This document explains the engine **from first principles**, starting with vocabulary and gradually building up to the full phase-level dispatch table.

---

## Design Goals

The engine is designed to satisfy the following constraints:

1. **Zero runtime overhead**

   * All tables are `constexpr` and fully materialized at compile time.
   * Runtime code performs only indexed lookups and function calls.

2. **Deterministic structure**

   * All indices are stable across translation units.
   * Table layouts depend only on type lists and value lists.

3. **Header-only**

   * Safe to include in any number of translation units.
   * No ODR issues, no global state.

4. **Strict layering**

   * Each registry layer has a single responsibility.
   * Higher layers build on lower ones without circular dependencies.

5. **Explicit contracts**

   * Concept authors must implement a precise compile-time interface.
   * Violations fail at compile time.

---

## Core Vocabulary

All registries share a common vocabulary defined in `callbacksRegistry_common.h`.

### Pipeline Dimensions

The encoding pipeline is defined along two fixed axes:

* **Stages** (`NUM_STAGES`)
* **Sections** (`NUM_SECTIONS`)

These values are compile-time constants and are used to dimension all dispatch tables.

Stages represent *when* an operation occurs (e.g. allocation, preset, runtime), while sections represent *where* in the GRIB message the operation applies.

### Sentinel Values

One sentinel constant is used throughout the engine:

* `MISSING`

It carries different semantic meaning depending on context. They are used to represent:

* absent indices,
* invalid lookups,
* structurally inapplicable situations.

### Function Pointer Types

Two canonical function pointer types are used:

* `Fn<...>` — encoding / transformation callbacks
* `Fm<...>` — matcher / predicate callbacks

All dispatch tables store one of these two types (or `nullptr`).

### Compile-Time Containers

Two lightweight compile-time containers are used everywhere:

* `ValueList<Vs...>` — a list of compile-time values (usually enum values)
* `TypeList<Ts...>` — a list of types

They provide *ordering*, *cardinality*, and *structure*, but no runtime behavior.

---

## The Entry Descriptor Contract

Every concept that participates in the registry engine must provide a **descriptor** conforming to `RegisterEntryDescriptor`.

Conceptually, an *Entry* represents:

* one semantic concept (e.g. level type, grid type),
* a finite set of variants,
* a family of dispatch functions at different granularities.

An Entry descriptor specifies:

1. **Variant enum type**
2. **Ordered list of variants** (`VariantList`)
3. **Concept name** (`entryName()`)
4. **Variant names** (`variantName<V>()`)
5. **Dispatch interfaces**

The dispatch interfaces are layered:

* entry-level (`entryCallbacks`)
* variant-level (`variantCallbacks`)
* phase-level (`phaseCallbacks`)

Each interface is selected by:

* a compile-time capability ID,
* optional variant, stage, and section indices,
* concrete dictionary types.

Returning `nullptr` means *not supported*.

---

## Registry Layers (Bottom-Up)

The compile-time registry engine is composed of several layers. Each layer builds on the previous one.

---

### 1. EntryVariantRegistry

**Purpose:** define the *index space*.

This registry computes:

* the total number of concepts,
* the total number of variants,
* the mapping between:

  * concept → offset
  * variant → local index
  * variant → global index

It also materializes constexpr tables:

* `conceptIdArr[globalVariant]`
* `variantIdArr[globalVariant]`
* `conceptNameArr[globalVariant]`
* `variantNameArr[globalVariant]`

This layer contains **no behavior**. It is purely structural and arithmetic.

Think of it as the *schema* of the system.

---

### 2. EntryCallbacksRegistry

**Purpose:** entry-level behavior.

This registry builds a table:

```
entryCallbacks[entryIndex] -> Fm | nullptr
```

Each entry contributes exactly one slot.

This layer answers questions like:

* “Does this concept support capability X?”
* “Which matcher applies for this dictionary pair?”

Granularity: **per concept**.

---

### 3. VariantCallbacksRegistry

**Purpose:** variant-level behavior.

This registry refines dispatch to the variant level:

```
variantCallbacks[globalVariant] -> Fn | nullptr
```

Each variant contributes exactly one slot.

Granularity: **per variant**.

This layer is typically used when:

* behavior depends on the chosen variant,
* but not on stage or section.

---

### 4. PhaseCallbacksRegistry (Final Layer)

**Purpose:** full phase resolution.

This registry builds the final, fully expanded dispatch table:

```
phaseCallbacks[globalVariant][stage][section] -> Fn | nullptr
```

This table answers the question:

> “For this variant, at this stage, in this section — what do I do?”

All dimensions are fixed at compile time.

This is the **final product** consumed by the encoding pipeline.

---

## Capability Model

A *capability* is represented by a compile-time `std::size_t` constant.

Capabilities are not enums by design:

* they are used as non-type template parameters,
* they index families of dispatch tables,
* they allow separate compilation of unrelated behaviors.

Each capability produces a **distinct instantiation** of every registry layer.

There is no runtime capability switching.

---

## How Everything Fits Together

1. Concepts define Entry descriptors.
2. EntryVariantRegistry defines the index space.
3. EntryCallbacksRegistry defines per-concept behavior.
4. VariantCallbacksRegistry defines per-variant behavior.
5. PhaseCallbacksRegistry defines full phase behavior.
6. Runtime code:

   * computes indices once,
   * indexes into constexpr tables,
   * calls function pointers.

No maps. No conditionals. No polymorphism.

---

## Mental Model

You can think of the engine as a **compile-time code generator** written in C++ templates.

At runtime, the system behaves as if someone had manually written:

* giant `switch` statements,
* fully unrolled loops,
* precomputed dispatch matrices.

Except:

* it is guaranteed correct by the type system,
* it is impossible to forget a case silently,
* and the compiler optimizes it aggressively.

---

## When to Use (and Not Use) This Engine

Use it when:

* the domain is finite and known at compile time,
* performance matters,
* behavior is highly structured.

Do **not** use it when:

* the domain is dynamic or user-extensible at runtime,
* configurability matters more than performance,
* compile times are a critical concern.

---

## Final Notes

This engine trades:

* longer compile times,
* heavier template instantiations,

for:

* minimal runtime overhead,
* maximal structural correctness,
* explicit, auditable behavior.

It is intentionally **not clever at runtime**.

All the cleverness happens at compile time.
