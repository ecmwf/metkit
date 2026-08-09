<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# ProductTimeSpec Constraints

## Purpose

This file lists the hard constraints that must not be violated while executing
the rewrite defined in `Requirements.md`.

This file is not a task list.

## Operational Constraints

- Never build.
- Never debug by running the code.
- Never commit.
- The user builds, debugs, and commits.
- While the code is under testing, touch only markdown unless explicitly asked
  to do otherwise.
- Do not change code or code behavior unless explicitly requested in a later
  implementation step.

## Scope Constraints

- During the markdown cleanup phase, touch only markdown files.
- The living markdown set for this subtree is:
  - `Requirements.md`
  - `Status.md`
  - `Constraints.md`
- All other previous markdown in this subtree is considered temporary and
  outdated.

## Callback Review Constraints

- No callback may be modified blindly.
- Every callback must be reviewed one by one before modification.
- The callback inventory in `Requirements.md` is the review checklist.
- Callback logic should become leaf logic.
- Hidden semantic control flow should be minimized.
- Small duplication is acceptable when it improves readability, reviewability,
  and robustness.

## Code Style Constraints

- Be consistent with the existing repository style.
- Be consistent with the existing style already present in this subtree.
- Code must be readable.
- Prefer explicit local logic over compressed abstraction when the latter hides
  semantics.
- Preserve readable naming and stable control flow.

## Namespace and `using` Constraints

- Every function that is not in the current namespace must be explicitly brought
  in with a `using` declaration.
- The `using` declaration must expose the full namespace path.
- Do not use shortened partial namespace paths in those `using` declarations.
- Follow the pattern already extensively used in the codebase.

## Shape Callback Structure Constraints

For shape window-building callbacks, the preferred structure is mandatory.

The callback must be organized as follows:

1. input validation;
2. one compact block of code for each member of `ProductTimeSpecWindow`;
3. members validation;
4. construction of the final window from the local variables already assigned
   and explained.

When multiple windows are needed:

- the same logic must be applied inside a loop;
- each produced window must still follow the same field-by-field structure.

## Per-Member Documentation Constraints

- Every compact block that computes one member of `ProductTimeSpecWindow` must
  be documented.
- The documentation must explain why that field is computed that way for that
  specific case.
- The code must be reviewable case by case without jumping through helper
  layers.
- Window construction should happen only after the local member variables are
  already assigned and explained.

## Data Model Constraints

- Keep the current absolute domain datetimes.
- Extend `ProductTimeSpecDomain` with:
  - `bool isSynoptic`
  - `long startOffsetHoursFromReference`
  - `long endOffsetHoursFromReference`
- Synoptic semantics must remain explicitly representable.
- The raw model must preserve enough information for reconstruction,
  comparison, normalization, and cross-checking.

## Pipeline Constraints

- Input-policy facts may be precomputed in `ProductTimeSpecInput` when doing so
  reduces callback-local recomputation and improves callback readability.
- Normalization must happen after raw callbacks.
- Per-callback raw checks must happen before normalization.
- Cross anchor/domain/shape checks must happen after normalization.
- Month-based raw semantics must remain raw until the exact placed interval is
  available.

## Input Contract Constraints

- The step-zero statistical-processing allowance may be represented as a
  dedicated derived member of `ProductTimeSpecInput`.
- If `innerMostTypeOfStatisticalProcessing` is
  `TypeOfStatisticalProcessing::Missing`, the step-zero allowance must evaluate
  to valid because it represents the instant case.
- Zero-step shape callbacks should consume the derived input fact rather than
  recomputing the policy locally once that member exists.

## Unit Constraints

- Final encoder-facing time ranges must be in hours.
- Final encoder-facing time increments must be in seconds or missing.
- Monthly hour values must be derived from the exact placed interval, not from
  a fixed approximation.
- Sub-monthly normalized hour validation source of truth is the language
  timespan support set.
- Monthly normalized hour validation source of truth is the ecCodes month-length
  concept space represented by:
  - `672`
  - `696`
  - `720`
  - `744`

## Error-Handling Constraints

- All diagnostic JSON used in exception enrichment must be best-effort.
- All diagnostic JSON used in exception enrichment must be `noexcept`.
- Diagnostic JSON must never throw while building exception context.
- All other nontrivial code must wrap the full body in `try/catch` and rethrow
  with nested context.

## Helper Layer Constraints

- `TemporalArithmetic` is the main allowed shared semantic helper layer.
- Other helper layers should be reduced when they hide too much callback logic.
- Helper extraction is acceptable only when it preserves or improves
  reviewability.

## Documentation Constraints

- In-code documentation must be improved after the implementation stabilizes.
- Markdown beyond the living files should be rewritten only after the code shape
  stabilizes.
- License headers must remain consistent.
