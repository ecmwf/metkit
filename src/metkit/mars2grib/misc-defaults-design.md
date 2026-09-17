# Misc Defaults Server: Design and Restart Context

## Scope and constraints

This analysis is confined to `backend/deductions` and proposes a new isolated
area under `backend/misc-defaults`.

The objective is to make all semantics associated with the misc/parameter
dictionary explicit and centrally discoverable. Deductions must not inspect
misc presence or implement misc defaults themselves.

The intended invariant is:

> Deduction code never calls `has`, `get_opt`, or `get_or_throw` directly on
> the misc dictionary. All misc access passes through the misc-defaults API.

## Current-state inventory

Within `backend/deductions`, `par` is the misc/parameter dictionary.

- 61 physical accesses occur across 27 headers.
- 35 distinct physical misc keys are accessed.
- All current access uses `dict_traits::{has, has<T>, get_opt<T>,
  get_or_throw<T>}`.
- There are no direct `operator[]`, `.at()`, `.find()`, or access macros.

### Mandatory values without defaults

- `numberOfComponents`
- `numberOfFourierCoefficients`
- `satelliteSeries`
- `modelErrorType`
- custom `tablesVersion`
- `scaledValueOfCentralWaveNumber`
- `scaleFactorOfCentralWaveNumber`

### Optional values that may remain absent

- `generatingProcessIdentifier`
- `iTmin`
- `iTmax`
- `lengthOfTimeWindow`
- `totalNumberOfIterations`
- `timeIncrementInSeconds`

### Fixed defaults currently implemented in deductions

| Physical key | Current fallback |
|---|---:|
| `numberOfForecastsInEnsemble` | `51` |
| `subCentre` | `0` |
| `numberOfFrequencies` | `54` |
| `scaleFactorOfWaveDirections` | `2` |
| `scaleFactorOfWaveFrequencies` | `6` |
| `shapeOfTheEarth` | spherical Earth with radius 6371229 |
| spectral `bitsPerValue` | `16` |
| `pvSize` | `137`, when neither `pv` nor `pvSize` is supplied |

### Values currently derived from MARS when misc is absent

- `derivedForecast`, from MARS `type`
- `typeOfEnsembleForecast`, from MARS `type`
- `typeOfProcessedData`, from MARS `class` and `type`

### Values currently derived from MARS and options

- gridded `bitsPerValue`, from `param`, `levtype`, and compression options
- `subSetTruncation`, from truncation/grid and `skipSection3`

### Composite misc configurations

- Wave directions can be supplied as `waveDirections` or reconstructed from
  `numberOfWaveDirections`.
- Wave frequencies can be supplied as `waveFrequencies` or reconstructed from
  `numberOfWaveFrequencies`, `indexOfReferenceWaveFrequency`,
  `referenceWaveFrequency`, and `waveFrequencySpacingRatio`.
- A PV array can be supplied directly as `pv`, indirectly as `pvSize`, or
  defaulted through `pvSize=137`.

These are logical properties with multiple representations, not independent
misc keywords.

### Important inconsistencies

- `bitsPerValue` has different gridded and spectral behavior despite sharing
  one physical key.
- `timeIncrementInSeconds` has two incompatible implementations:
  - `timeIncrement.h` accepts `long` or `string`, and rejects zero.
  - `timeIncrementInSeconds.h` accepts only `long`, and treats zero as absent.
- `typeOfGeneratingProcess.h` contains currently unreachable misc accesses due
  to an earlier unconditional return.
- Some deduction documentation is stale, notably for `shapeOfTheEarth` and
  `numberOfForecastsInEnsemble`.

## Semantic distinction required by the API

The interface must distinguish real misc defaults from genuine deductions that
can merely be overridden through misc.

```cpp
enum class MiscPolicy {
    Required,
    Defaulted,
    Optional,
    OptionalOverride
};
```

- `Required`: misc must provide one valid representation; there is no default.
- `Defaulted`: misc may provide a representation; otherwise the misc server
  supplies an actual default.
- `Optional`: absence is a valid final result.
- `OptionalOverride`: absence is expected and delegates to a genuine deduction.

For example, the MARS mapping for `derivedForecast` is probably a genuine
deduction, not the default value of a misc keyword. Therefore the misc value is
an `OptionalOverride`.

By contrast, `subCentre=0` is a real misc default and belongs to `Defaulted`.

## Proposed isolation boundary

```text
backend/
`-- misc-defaults/
    |-- common.h
    |-- resolve.h
    |-- catalogue.h
    |-- recording.h
    |-- describe.h
    |-- rules/
    |   |-- scalar.h
    |   |-- pvArray.h
    |   |-- waveDirectionGrid.h
    |   `-- waveFrequencyGrid.h
    `-- detail/
        `-- dictionaryAccess.h
```

Only `detail/dictionaryAccess.h` should call low-level dictionary operations on
misc. Deductions use `resolve.h`; recording and help tooling use the catalogue
and description interfaces.

## Proposed deduction-facing API

```cpp
auto value = resolve_misc_or_throw<rules::PvArray>(
    misc, mars, opt, cntx);

auto override = resolve_misc_opt<rules::DerivedForecast>(
    misc, mars, opt, cntx);
```

Possible naming remains open, but the two operations mean:

- `resolve_misc_or_throw`: produce a final value from an explicit misc
  representation or a real misc default; throw if impossible.
- `resolve_misc_opt`: return an optional value for `Optional` or
  `OptionalOverride` policies.

The policy should be identified by a compile-time logical rule rather than only
by a runtime physical key. A key-only API cannot distinguish gridded and
spectral `bitsPerValue` or conflicting temporal policies.

The API should enforce policy-compatible calls at compile time:

- `resolve_misc_or_throw` accepts `Required` and `Defaulted` rules.
- `resolve_misc_opt` accepts `Optional` and `OptionalOverride` rules.

## Result and provenance

Resolution should retain how the value was obtained so deductions do not need
to recreate misc presence logic for logging.

```cpp
enum class MiscSource {
    Explicit,
    Default
};

template <class T>
struct ResolvedMisc {
    T value;
    MiscSource source;
    std::string_view representation;
};
```

The representation identifies the accepted input form used, such as `pv` or
`pvSize`.

For an optional override, deduction code may legitimately branch:

```cpp
if (auto override = resolve_misc_opt<rules::DerivedForecast>(
        misc, mars, opt, cntx)) {
    return convert_override(override->value, cntx);
}

return deduce_derived_forecast(mars, cntx);
```

This branch is genuine deduction selection, not hidden misc default logic.

## Logical properties and representations

A rule represents one logical property regardless of the number of physical
keys that can specify it.

```cpp
enum class MiscProperty {
    PvArray,
    DerivedForecast,
    BitsPerValueGridded,
    BitsPerValueSpectral,
    WaveFrequencyGrid
    // ...
};
```

A physical key is therefore not the identity of a rule:

- `pv` and `pvSize` are representations of `PvArray`.
- The five wave-frequency keys describe two representations of one logical
  `WaveFrequencyGrid` input.
- Gridded and spectral rules share the `bitsPerValue` physical key but have
  distinct logical identities and policies.

## PV-array example

The deduction should make one call:

```cpp
auto pv = resolve_misc_or_throw<rules::PvArray>(misc, mars, opt, cntx);
```

Its server-side contract is:

```text
Logical property: pvArray
Relevant GRIB key: pv
Final result: required vector<double>
Policy: defaulted

Representations:
  pv:
    input type: array of real numbers
    conversion: identity

  pvSize:
    input type: integer
    constraint: one of the supported PV table sizes
    conversion: PV table lookup

Combination:
  pv and pvSize are mutually exclusive

Default:
  representation: pvSize
  value: 137
```

Behavior:

| `pv` | `pvSize` | Result |
|---|---|---|
| present | absent | Use the supplied array |
| absent | present | Look up the array by size |
| absent | absent | Look up the array using `pvSize=137` |
| present | present | Throw an ambiguity error |

All presence checks, representation selection, transformation, defaulting, and
combination validation belong to the server.

## Declarative contracts

The server needs machine-readable metadata for recording and help generation.

```cpp
struct MiscRepresentation {
    std::string_view key;
    ValueType inputType;
    std::string_view conversion;
    ValueConstraint constraint;
};

struct MiscContract {
    MiscProperty property;
    std::string_view logicalName;
    std::string_view gribKey;
    MiscPolicy policy;
    std::vector<MiscRepresentation> representations;
    CombinationRule combination;
    DefaultDescription defaultDescription;
};
```

Default metadata should distinguish:

```cpp
enum class DefaultKind {
    None,
    Fixed,
    MarsDependent,
    OptionsDependent,
    MarsAndOptionsDependent
};
```

The exact concrete representation is not settled. The important requirement is
that runtime resolution and generated documentation derive from the same
authoritative rule, avoiding duplicated descriptions.

## Catalogue

The catalogue is the authoritative index of logical properties:

```cpp
const MiscContract& contract(MiscProperty);
std::span<const MiscContract> all_contracts();
```

It must answer:

- Which logical misc properties exist?
- Which physical keys represent each property?
- Which GRIB key or keys are affected?
- What policy applies?
- Which types and constraints are accepted?
- Which representations are alternatives or groups?
- What happens when all representations are absent?

Complex implementation may remain in individual rule files, while the
catalogue exposes a uniform contract.

## Recording mode

Recording should collect the logical property, not merely physical keys:

```cpp
record(MiscProperty::PvArray);
```

Recording `pv` and `pvSize` independently would lose their relationship as
alternative representations of the same canonical result.

A minimal event is:

```cpp
struct RecordedMiscAccess {
    MiscProperty property;
    SourceLocation callSite;
};
```

The resolver detects a recording dictionary through a trait or customization
point and records the rule before attempting ordinary lookup.

Suggested continuation behavior in recording mode:

- `Defaulted`: evaluate and return the real default.
- `OptionalOverride`: return `std::nullopt`, allowing the genuine deduction to
  run and collect its other relevant properties.
- `Optional`: return `std::nullopt`.
- `Required`: return a centrally declared synthetic exemplar so collection can
  continue rather than stopping at the first missing mandatory value.

A recording exemplar is not a default and must never be presented as one:

```cpp
static value_type recording_exemplar(...);
```

The recording dictionary should preserve logical access order if useful, while
the later description stage can deduplicate properties.

## Post-recording mapping and help generation

After encoding, a separate stage maps recorded logical properties to relevant
GRIB keys and obtains their full contracts from the catalogue:

```cpp
auto properties = recorder.collected_properties();

auto help = describe_misc_requirements(
    properties, mars, opt, cntx);
```

This stage should:

1. Deduplicate logical properties.
2. Look up each property in the catalogue.
3. Map the property to the relevant GRIB key or keys.
4. Evaluate request-dependent defaults for the supplied MARS/options context.
5. Present accepted representations, types, constraints, combinations, and
   defaults using coherent grammar or tables.

Example PV output:

| GRIB key | Misc input | Type | Requirement |
|---|---|---|---|
| `pv` | `pv` | Array of real values | Cannot be combined with `pvSize` |
| `pv` | `pvSize` | Supported integer PV size | Cannot be combined with `pv` |
| `pv` | Neither | None | Uses `pvSize=137` |

Possible generated prose:

> GRIB key `pv` can be specified through `misc["pv"]` as an array of real
> values, or through `misc["pvSize"]` as one of the supported integer PV
> sizes. These representations are mutually exclusive. If neither is supplied,
> `pvSize=137` is used.

For contextual defaults, both static and evaluated information should be
available. For example, help for gridded `bitsPerValue` could explain its MARS
and option dependencies and also state the value calculated for the current
request.

## Preliminary policy classification

This classification encodes architectural intent and requires review before
migration.

### Likely `Defaulted`

- `subCentre`
- `shapeOfTheEarth`
- `numberOfForecastsInEnsemble`
- `numberOfFrequencies`
- Wave scale factors
- `pvArray`

### Likely `OptionalOverride`

- `derivedForecast`
- `typeOfEnsembleForecast`
- `typeOfProcessedData`
- Gridded and spectral `bitsPerValue`
- `subSetTruncation`

The last two currently call their fallback a default, but the fallback may be a
genuine domain deduction rather than misc defaulting. This distinction must be
decided deliberately.

### Likely `Required`

- `satelliteSeries`
- `modelErrorType`
- `numberOfComponents`
- `numberOfFourierCoefficients`
- Custom `tablesVersion`
- Wave grids until genuine defaults are defined

### Likely `Optional`

- `generatingProcessIdentifier`
- `iTmin`
- `iTmax`
- `lengthOfTimeWindow`
- `totalNumberOfIterations`

## Proposed migration sequence

1. Add the isolated `backend/misc-defaults` core types and APIs.
2. Define required, defaulted, optional, and optional-override semantics.
3. Implement simple scalar rules first.
4. Migrate scalar deductions and prohibit direct low-level misc access.
5. Implement `PvArray` as the first multi-representation rule.
6. Add wave-direction and wave-frequency composite rules.
7. Resolve context-sensitive and polymorphic cases.
8. Add logical-property recording.
9. Add catalogue-based GRIB mapping and help generation.
10. Verify that deductions contain no direct misc `has`, `get_opt`, or
    `get_or_throw` calls.

## Open design questions

- Final names for `resolve_misc_or_throw` and `resolve_misc_opt`.
- Whether `opt` is an explicit API argument. It is currently recommended
  because some behavior depends on options.
- The exact compile-time versus runtime shape of rule descriptors.
- How one rule generates both executable resolution and declarative metadata
  without duplicating logic.
- How constraints such as supported PV sizes are exposed to help generation.
- How required recording exemplars are chosen without accidentally passing
  unrelated semantic validation.
- Whether logging belongs in the server or remains in deductions using returned
  provenance.
- Final policy classifications, especially `bitsPerValue`,
  `subSetTruncation`, and wave-grid inputs.
- How to reconcile the two `timeIncrementInSeconds` semantics.
- Whether one logical property may map to multiple GRIB keys and how that is
  represented in the catalogue.

## Central design conclusions

1. Misc presence checks must disappear from deductions.
2. The server owns retrieval, representation selection, misc defaulting, and
   representation constraints.
3. Real misc defaults and optional overrides of genuine deductions are
   different policies.
4. Rules identify logical properties, not merely physical dictionary keys.
5. Composite inputs such as PV and wave grids are represented by one logical
   contract with multiple input forms.
6. Recording collects logical rule identifiers.
7. A later catalogue-driven stage maps rules to GRIB keys and produces detailed
   help.
8. Runtime behavior and generated help must use the same authoritative contract.
