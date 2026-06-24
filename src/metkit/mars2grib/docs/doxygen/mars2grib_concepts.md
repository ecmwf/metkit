@page concepts_page Concepts

@section concepts_definition Definition of a Concept

A **Concept** is the fundamental semantic unit of the Encoder.

A concept represents an orthogonal piece of information required to describe
an encoded product. Concepts are the only entities that **set values in the
GRIB header**.

@image html concept.svg "General idea of a concept"

Concepts do not decide *what* values to set. They only decide *where* and *how*
values are written. All value resolution is delegated to deductions.

@section concepts_compile_time Compile-Time Registration Model

Concepts are registered entirely at compile time.

Each registered entity is a **pair** composed of:

1. a **key**, identifying the semantic capability
2. a **value**, representing the executable contributions

@subsection concepts_registration_key Registration Key

The registration key is:

Concept::Variant


Each `(Concept, Variant)` pair represents a distinct semantic realization and
is treated as an independent entity by the Encoder.

@section concepts_registration_value Registration Value

The value associated with each `Concept::Variant` key is a **dense, fixed-size
table** indexed by:

- **Stage**
- **Section**

Conceptually:

Table[Stage][Section] -> function pointer | nullptr


Each entry corresponds to a specialization of the concept for a given
`(Stage, Section)` pair.

- A non-null entry indicates that the concept provides logic for that
  `(Stage, Section)`.
- A null entry indicates that no specialization exists for that pair.

This table is constructed at compile time and is immutable.

@section concepts_specialization Specializations

A concept provides logic by defining one or more specializations of the form:

Concept<Variant, Stage, Section>


Only explicitly defined specializations are registered.
No implicit instantiation or fallback mechanism exists.

Missing `(Stage, Section)` combinations are represented by null pointers in
the registration table.

@section concepts_dispatch_role Role in Encoder Dispatch

The compile-time registration tables define the **maximum set of capabilities**
available to the Encoder.

At runtime:
- the EncoderConfiguration selects a set of `(Concept, Variant)` keys
- for each selected key, only the non-null `(Stage, Section)` entries are
  extracted
- all null entries are explicitly discarded

From this process, a **dense runtime dispatch table** is built, containing
only valid function pointers.

Concepts are unaware of:
- which other concepts are active
- execution order
- template numbers
- runtime configuration

@section concepts_deductions Deductions as the Only Resolution Mechanism

Concepts do not access input dictionaries directly.

All resolution of GRIB keywords and values is performed exclusively through
**deductions**.

A deduction:
- reads information from the MARS and misc dictionaries
- applies validation, normalization, and consistency rules
- exposes resolved values through a stable interface

Concepts consume only deduction results and never inspect raw inputs.

This strict separation ensures that:
- concepts remain dictionary-agnostic
- resolution logic is centralized
- concepts are reusable and independently testable

@section concepts_orthogonality Semantic Orthogonality

Concepts are **semantically orthogonal**.

Each concept:
- has a single, well-defined semantic responsibility
- does not depend on other concepts
- does not encode cross-concept assumptions

Any interaction between concepts is expressed through:
- deduction logic
- recipe constraints

Never through direct concept-to-concept coupling.

@section concepts_non_goals Explicit Non-Goals

Concepts intentionally do not:
- select template numbers
- perform value deduction
- allocate memory
- manage execution order
- inspect runtime configuration

Concepts are purely declarative, statically registered contributors to the
encoding process.

