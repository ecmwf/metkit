@page architecture_page Encoder Architecture

@section arch_scope Scope and Inputs

The Encoder is designed to operate on a minimal and well-defined set of inputs.
The user is expected to provide:

- a **MARS dictionary**
- a **miscellaneous dictionary** (hereafter referred to as *misc dictionary*)

No additional dictionaries are required or assumed.

All information required to determine the encoded product structure is derived
from the inputs listed above and from internal compile-time rules.

@section arch_mars_dictionary The MARS Dictionary

The MARS dictionary is the primary semantic input of the Encoder.

In addition to classical MARS metadata, the dictionary is **extended** and may
contain post-processing related keywords, including (but not limited to):

- grid
- truncation
- packing

These extended keywords are treated as first-class inputs and participate
directly in the encoding logic.

@section arch_split Frontend and Backend Separation

The Encoder is logically split into two distinct components:

@subsection arch_frontend Frontend

The frontend:
- depends **only** on the MARS dictionary
- performs no encoding
- produces an intermediate object called **EncoderConfiguration**

The frontend is responsible for interpreting the semantic intent expressed
by the MARS dictionary and mapping it to an internal structural description.

@subsection arch_backend Backend

The backend:
- depends on the MARS dictionary
- depends on the misc dictionary
- depends on the EncoderConfiguration produced by the frontend

The backend performs the actual encoding by executing the encoding logic
described by the configuration.

@image html architecture.svg "General overview of the encoder architecture"

@section arch_encoder_configuration EncoderConfiguration

The EncoderConfiguration is a complete, self-contained description of the
**structure** of the encoded header.

Here, *structure* refers to:
- memory layout
- presence of keywords
- association between sections and concepts

The EncoderConfiguration is specified as follows.

@subsection arch_sections Sections and Template Numbers

For each encoding section, a template number is provided.

- For sections 2, 3, 4, and 5, the template number is semantically relevant.
- For all other sections, the template number is present but effectively
  a dummy value, as only a single valid template exists.

@subsection arch_concept_variants Concept Variants per Section

For each section, the EncoderConfiguration contains an ordered list of
`Concept::Variant` entries.

Each entry identifies:
- the concept
- the concrete variant of that concept to be used in that section

@section arch_configuration_construction Construction of EncoderConfiguration

The EncoderConfiguration is constructed by combining:

- information extracted from the MARS dictionary
- compile-time information provided by recipes

The construction process follows a strict and deterministic workflow.

@subsection arch_step1 Concept Activation

From the MARS dictionary:
- the set of **active concepts** is determined
- for each active concept, a **version** is deduced

Inactive concepts are excluded at this stage.

@subsection arch_step2 Template Determination

The set of active `(Concept, Version)` pairs is used to determine:
- the template number for each relevant section

This step must result in a unique and consistent set of template numbers.

@subsection arch_step3 Recipe Completion

Once template numbers are known, recipes are used to complete the
EncoderConfiguration.

For each `(section, template number)` pair, a recipe provides a list of
`(Concept, Variant)` pairs.

The following rules apply:

- If a variant is **not explicitly specified** in the recipe, it is assumed
  to be `Default`.
- If a variant is explicitly specified in the recipe, it is **mandatory**.
  A mismatch with the variant deduced from the MARS dictionary results in
  an error.
- If the recipe specifies `Default`, the variant deduced from the MARS
  dictionary may override it.

@section arch_dispatch Dispatch Table Construction

Once the EncoderConfiguration is finalized, a three-dimensional table of
function pointers is constructed.

The table is indexed as:

- **Stage**
- **Section**
- **Concept**

The table is inferred **exclusively** from the EncoderConfiguration.
No additional runtime decisions are required.

During construction, **all null entries are explicitly eliminated**.
Only valid `(Stage, Section, Concept)` combinations are inserted.

As a result, the dispatch table is **dense** and contains only callable
function pointers.

@section arch_execution Encoding Execution

Encoding is performed by iterating over the dispatch table.

- for each stage:
  - for each section:
    - all function pointers associated with that stage and section are invoked
      in deterministic order

Because the dispatch table contains no null entries:
- execution is purely linear over the precomputed table

All semantic decisions are completed prior to execution, during the
construction of the EncoderConfiguration and the dispatch table.
