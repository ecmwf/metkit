# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

- **Breaking (pymetkit):** `ParamDB` short name lookups (`shortname_to_param_id`, `shortname_to_param_id_candidates`, `shortname_to_longname`) now require a `context` with non-empty values for all of `class`, `stream`, `type` and `levtype`, and raise `ValueError` otherwise. Previously, missing keys were silently filled with MARS defaults, which could return the wrong param id (e.g. `tp` with `{"class": "ai"}` gave 228 instead of 228228). `context={}` is now rejected; omit `context` (`context=None`) to keep the previous behaviour of listing all candidates.

[Unreleased]: https://github.com/ecmwf/metkit/compare/1.22.0...HEAD
