# Copilot Repository Instructions

## Documentation synchronization rule

When working on a pull request:

1. Determine the set of files modified by the PR.
2. From that set, consider only files under:

   - src/metkit/mars2grib
   - test/mars2grib

3. For each of those files:
   - Verify that the related documentation is present
   - Verify that the documentation is up to date with the code changes
   - If documentation is missing or outdated, propose the required updates

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