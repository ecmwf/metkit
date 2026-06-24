@page mars2grib_deductions Deductions

Deductions are small, single-purpose components used by the MARS-to-GRIB backend
to resolve GRIB keys from input metadata. Each deduction reads specific keys from
one or more dictionaries and deterministically produces a single GRIB value
(or an optional value when the mapping is intentionally conservative).

Each deduction is responsible for **exactly one GRIB key**. This one-to-one
mapping makes the resolution logic explicit, traceable, and easy to audit.

@section mars2grib_deductions_role Role in the pipeline

Deductions sit between input metadata (MARS, parameter, and options dictionaries)
and GRIB encoding. Their responsibilities are intentionally narrow:

- Extract required inputs from dictionaries.
- Apply explicit, deterministic mapping rules.
- Return resolved values or raise a detailed error when resolution is not possible.
- Emit structured diagnostic logs describing how a value was obtained.

Deductions do **not**:

- Infer missing values unless explicitly defined.
- Apply silent fallbacks.
- Validate physical or semantic correctness beyond documented rules.
- Rely on pre-existing GRIB header state.

@section mars2grib_deductions_inputs Input dictionaries

Most deductions operate on these dictionary types:

- **MARS dictionary**: request metadata (e.g. @c type, @c stream, @c param, @c timespan).
- **Parameter dictionary**: user overrides or explicit encoding choices
  (e.g. @c tablesVersion, @c typeOfProcessedData).
- **Options dictionary**: configuration flags controlling optional behavior.

Unless a deduction explicitly declares a default or an optional mapping,
required inputs must be present.

@section mars2grib_deductions_errors Error handling

Deductions follow a fail-fast strategy. When a required input is missing or invalid,
the deduction throws a nested exception that preserves context. This makes failures
traceable while avoiding silent behavior changes.

@section mars2grib_deductions_logging Logging policy

Deductions emit structured logs for traceability. Typical policy categories include:

- **RESOLVE**: value derived from input dictionaries or deterministic mappings.
- **OVERRIDE**: value explicitly set by the parameter dictionary.
- **DEFAULT**: value supplied by a documented default rule.

Logging statements must match the actual path taken in the code.

@section mars2grib_deductions_debugging Debugging advantages

The one-to-one relationship between a deduction and a GRIB key significantly
simplifies debugging.

Most error reports have the form:

@code
grib key "xxx" has the wrong value, it is supposed to be "a" instead is "b"
@endcode

Because each GRIB key is produced by a single, well-identified deduction:

- The faulty component is immediately known from the key name.
- The resolution path is localized and deterministic.
- The input dictionaries and mapping rules involved are limited and explicit.
- The structured logs describe exactly how the incorrect value was obtained.

In practice, this makes it almost immediate to identify the logic that generated
the error and to reproduce the issue.

@section mars2grib_deductions_determinism Determinism and reproducibility

Given the same inputs, deductions return the same outputs. Any dependence on
runtime state (e.g. reading ecCodes defaults) must be explicit and documented
in the deduction that performs it.

@section mars2grib_deductions_new How to add a new deduction

When implementing a new deduction:

1. Define the required inputs and where they are sourced.
2. Document mapping rules and defaults (if any).
3. Implement a single, deterministic resolution path.
4. Emit the correct log category based on how the value is obtained.
5. Throw a detailed exception on failure.
6. Bind the deduction to the single GRIB key it is responsible for.

@section mars2grib_deductions_summary Summary

Deductions are the authoritative mapping layer between MARS metadata and GRIB
encoding. They are intentionally strict, deterministic, and auditable, ensuring
reproducible behavior, clear diagnostics, and fast error localization across
the backend.