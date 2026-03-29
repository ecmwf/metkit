# Copilot Repository Instructions

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
