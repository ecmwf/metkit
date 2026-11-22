# Encoder specifications

This directory contains high-level, machine-readable specifications for
`mars2grib` encoding behaviour.

The specifications are intended to provide:

- traceability from product requirements to encoder behaviour;
- enough information to review concept and recipe changes;
- stable expectations that automated tools can compare with the implementation;
- representative examples without duplicating all implementation details.

## Normative sections

In each message specification, the following sections describe expected
behaviour:

- `mars`: the MARS metadata that identifies the message;
- `concepts`: expected active variants and explicitly inactive concepts;
- `structure`: expected GRIB section templates and their recipe concepts;
- `matching_logic`: semantic rules and constraints;
- `encoding`: how GRIB key values are obtained;
- `verification`: observable output and invariants.

The `description`, `notes`, `implementation`, and `mars.example` fields provide
context and traceability. They are not independent encoder requirements.

## Conventions

- Concept and variant names use their C++ names without numeric registry IDs.
- `required` means a MARS key must be present, not that every possible value is
  accepted.
- `absent` identifies keys whose presence would select a different case.
- `match` defines exact identifying values.
- `one_of` defines accepted sets of identifying values.
- Expected template numbers refer to GRIB template numbers, not recipe indexes.
- `expected_change: none` means the case documents existing behaviour.
- Project names are requirement metadata supplied by product owners; they are
  not inferred from encoder code.

The initial proposal is in `representative-fields.yaml`. It documents
instantaneous 2 metre temperature, accumulated total precipitation, and the
seasonal metadata rules that may be composed with supported fields.
