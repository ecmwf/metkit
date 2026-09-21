# Misc Defaults

This directory is the isolated authority for reading the misc dictionary and
describing how misc inputs affect GRIB keys.

The layer is intentionally not wired into the encoder yet. Existing deductions
continue to use their current implementations while this interface is reviewed.

## Boundaries

- Only `Access.h` performs low-level misc dictionary access.
- Each physical misc keyword has a rule header under `rules/`.
- Composite logical properties, including PV and wave grids, have an additional
  rule that selects between their physical representations.
- `Registry.h` is used only for help generation, not runtime dispatch.
- `Help.h` maps physical keys collected by a recording dictionary to structured
  help for the relevant GRIB keys.
- No code in this layer detects or treats a recording dictionary specially.

## Resolution

Callers eventually use one of two entry points:

```cpp
resolve_misc_or_throw<Rule>(misc, mars, opt, cntx);
resolve_misc_opt<Rule>(misc, mars, opt, cntx);
```

The first always produces a value or throws. The second represents an absence
behavior that intentionally returns no value, including optional overrides of
genuine deductions.

`MiscPolicy` is user-facing metadata:

- `Mandatory`: omission can produce an incorrect encoding. A fallback must
  nevertheless exist so requirements discovery can complete.
- `Optional`: omission is valid and uses a default or produces no value.
- `OptionalOverride`: omission is preferred and delegates to a genuine
  deduction.

Mandatory fallbacks that have not yet been agreed are explicit TODO exceptions.

## Recording and Help

A recording run supplies an empty dictionary whose normal dictionary-access
operations record requested physical keys. The misc-defaults layer remains
unaware that recording is taking place and follows its ordinary absence paths.

After encoding, `describe_recorded_misc()`:

1. Maps recorded misc keys to GRIB keys.
2. Groups keys that form alternative or multi-key representations.
3. Evaluates request-dependent defaults from MARS and options.
4. Returns `MiscHelp` records containing policy, alternatives, modifiers,
   accepted types, descriptions, and effective defaults.

For example, the GRIB `pv` property has two alternatives:

- `pv`, an array of real coefficients;
- `pvSize`, a supported integer table size.

If neither is supplied, the effective default is `pvSize=137`.

The current four-key wave-frequency reconstruction representation is retained
for this initial version. It may later be replaced by one array-valued misc key.
