# Copilot Instructions for mars2grib

These instructions apply to AI-assisted changes under `src/metkit/mars2grib`.

## Concept Or Variant Decision

Before implementing any modification in the mars2grib concept system, determine whether the requested behavior is a new concept or a new variant of an existing concept.

- Use a new concept when the feature is an independent semantic axis that must be composable with other concepts.
- Use a new variant when the feature is an alternative realization inside an existing semantic axis and does not need independent composability.
- This distinction is usually not reliably deducible from code structure alone.
- If the user has not explicitly said whether the change is a new concept or a variant, ask the user before implementing.

## Level Concept Guardrail

The `level` concept is intentionally constrained. In the GRIB header, vertical level information is ultimately represented by these six low-level fixed-surface keys:

- `typeOfFirstFixedSurface`
- `scaleFactorOfFirstFixedSurface`
- `scaledValueOfFirstFixedSurface`
- `typeOfSecondFixedSurface`
- `scaleFactorOfSecondFixedSurface`
- `scaledValueOfSecondFixedSurface`

Do not set these keys directly as a shortcut or workaround.

mars2grib encodes only official level definitions by relying on `typeOfLevel` plus, when needed, `level`, `topLevel`, `bottomLevel`, and PV-array data. Each supported `typeOfLevel` maps to a prescribed fixed-surface configuration. Some virtual `typeOfLevel` values exist because they cannot be introduced in ecCodes for backward-compatibility reasons.

If a requested change appears to require direct writes to fixed-surface keys, do not implement that approach. Instead, add or adjust the appropriate `LevelType` variant, matcher mapping, or deduction so the level remains encoded through the official level abstraction.


## Documentation synchronization rule

When working on a pull request:

1. Determine the set of files modified by the PR.
2. From that set, consider only files under:
   - `src/metkit/mars2grib`
   - Changes under `tests/mars2grib` require documentation updates only if they affect documented public behaviour

3. For each of those files:
   - Verify that the related documentation is present in the concrete doc locations for mars2grib, including (where applicable):
     - `src/metkit/mars2grib/docs/**`
     - Any module-level `.md` files or Doxygen pages associated with the modified code
   - Verify that the documentation in these locations is up to date with the code changes
   - If documentation is missing or outdated in these locations, propose the required updates

4. Do not request documentation changes for files outside these paths.

## Definition of “documentation in sync”

Documentation must:
- Describe the current public behavior and interfaces
- Reflect any new parameters, options, or outputs
- Remove references to deleted functionality

## PR review behavior

During PR reviews:
- Explicitly list the impacted files in the two target directories
- State whether documentation is:
  - ✅ in sync
  - ❌ missing
  - ❌ outdated
- Suggest concrete doc patches when needed, referencing the specific lines or sections that require updates.