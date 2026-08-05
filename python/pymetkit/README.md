# pymetkit

Python interface to the MetKit library for parsing MARS requests and looking up
ECMWF parameter metadata.

---

## MARS request parsing

`parse_mars_request` accepts a string or file-like object and returns a list of
`MarsRequest` instances.

### From a string

```python
from pymetkit import parse_mars_request

request_str = "retrieve,class=od,date=20240124,time=12,param=129,step=12,target=test.grib"
requests = parse_mars_request(request_str)

print(requests[0])
# verb: retrieve, request: {'class': ['od'], 'date': ['20240124'], ...}
```

### From a file

```python
from pymetkit import parse_mars_request

requests = parse_mars_request(open("test_requests.txt", "r"))
```

---

## ParamDB

`ParamDB` provides parameter ID ↔ shortname ↔ longname lookups backed by the
ECMWF parameter database.

### Quick start (offline, bundled data)

```python
from pymetkit import ParamDB

db = ParamDB()

db.shortname_to_param_id("msl")   # → 151  (unique shortname)
db.param_id_to_shortname(130)     # → "t"
db.shortname_to_longname("t")     # → "Temperature"
db.param_id_to_longname(130)      # → "Temperature"
db.get_units(130)                  # → "K"
db.get_metadata(130)               # → full metadata dict
```

### Collision resolution

Some shortnames map to more than one param ID (different GRIB tables,
originating centres, or MARS contexts). The resolver **never guesses**: if a
shortname is ambiguous, `shortname_to_param_id` raises `AmbiguousParamError`
carrying every candidate. Narrow the result with a MARS `context=` (resolved
through the C++ `expand` engine) and/or the `table=` / `origin=` / `access=`
hard filters.

The signature is:

```python
db.shortname_to_param_id(shortname, context=None, *, table=None, origin=None, access=None)
```

`context=` is a single dict of MARS keys. `table`, `origin` and `access` are
**scalar** hard filters passed individually (not a dict) — e.g.
`table=228, origin=98`.

```python
from pymetkit import ParamDB, AmbiguousParamError

db = ParamDB()

# No context argument → stays ambiguous → raises, error lists the candidates
try:
    db.shortname_to_param_id("tp")
except AmbiguousParamError as e:
    e.candidates
    # [ParamIDCandidate(param_id=228,    table=128, ..., mars_request_context={}),
    #  ParamIDCandidate(param_id=228228, table=228, ...,
    #                   mars_request_context={'class': 'ai', 'levtype': 'sfc',
    #                                         'type': 'fc'})]

# An *explicit* empty context resolves to the canonical/default id via expand
db.shortname_to_param_id("tp", context={})               # → 228

# Partial MARS context (funnelled through expand; usually enough)
db.shortname_to_param_id("tp", context={"class": "od"})  # → 228
db.shortname_to_param_id("tp", context={"class": "ai", "stream": "oper",
                                        "type": "fc", "levtype": "sfc"})  # → 228228

# Hard metadata filters (no MARS request built) — combinable with context=
db.shortname_to_param_id("tp", table=228)                # → 228228
db.shortname_to_param_id("t", origin=98)                 # → 130

# Programmatic discovery: every candidate + its selecting context
db.shortname_to_param_id_candidates("tp")
# [ParamIDCandidate(param_id=228, ...), ParamIDCandidate(param_id=228228, ...)]

# Raw candidate rows
db.get_all_by_shortname("tp")
# [{'id': 228, 'shortname': 'tp', ...}, {'id': 228228, 'shortname': 'tp', ...}]

db.shortname_has_collisions("tp")  # → True
db.shortname_has_collisions("msl") # → False (only one candidate)
```

#### Understanding `mars_request_context`

Each candidate advertises how to select it via its `mars_request_context`:

| Value | Meaning | How to select |
|-------|---------|---------------|
| `{}` | This is the **default** candidate | `context={}` resolves to it |
| `{...}` (non-empty) | Minimal distinguishing MARS context | pass it as `context=` |
| `None` | **No** MARS context can isolate it | use the hard filters instead |

When `mars_request_context is None`, use `candidate.hard_filter_selector` — a
ready-to-splat dict of `table=` / `origin=` / `access=` kwargs:

```python
try:
    db.shortname_to_param_id("sst")
except AmbiguousParamError as e:
    for c in e.candidates:
        if c.mars_request_context is None:            # e.g. id=151159 (table 151)
            db.shortname_to_param_id("sst", **c.hard_filter_selector)   # → 151159
        else:                                         # id=34, mars_request_context={}
            db.shortname_to_param_id("sst", context=c.mars_request_context)  # → 34
```

Across the full parameter database, of ~7,150 shortnames **97.7 % are
unambiguous**; of the 162 real collisions, **155 resolve** with
`context`/`table`/`origin`/`access`, and only **7** (e.g. `cdct`, `tcond`,
`swdi`, `ru-103`) are same-table/origin twins that no filter can separate.

> **Note:** `context=` is resolved by the C++ `expand` engine, which reads its
> language files from `~metkit/share/metkit`. Set `METKIT_HOME=<repo root>` to
> resolve against the in-repo `share/metkit`. A non-empty
> `mars_request_context` is the smallest MARS key-subset that actually
> round-trips through `expand` to that id; `context={}` maps to the canonical
> default. Without the C++ library the `context=` path is unavailable, but the
> `table`/`origin`/`access` hard filters still work.



### Online mode (live API + local cache)

```python
db = ParamDB(mode="online")                   # fetches from codes.ecmwf.int
db = ParamDB(mode="online", cache_ttl=timedelta(hours=6))  # custom TTL
db = ParamDB(mode="online", cache_path="/tmp/myapp")       # custom cache dir
```

---

## Custom parameter YAML

You can extend or replace the bundled database with your own YAML file.

### Loading a custom YAML

```python
db = ParamDB(yaml_path="my_params.yaml")
```

The file is loaded instead of the bundled `parameter_metadata.yaml`.
You can mix custom parameters alongside the bundled ones by loading in two
passes, but `ParamDB` does not merge files automatically — for that, concatenate
your YAML list with the bundled data before passing it in.

### YAML schema

Each entry is a YAML mapping. The fields are:

| Field        | Type           | Required | Default     | Description                                      |
|--------------|----------------|----------|-------------|--------------------------------------------------|
| `id`         | integer        | ✓        |             | Unique numeric param ID                          |
| `shortname`  | string         | ✓        |             | Short identifier (e.g. `"t"`, `"myvar"`)         |
| `longname`   | string         | ✓        |             | Human-readable description                       |
| `units`      | string         |          | `"unknown"` | Physical units (e.g. `"K"`, `"m s**-1"`)        |
| `origin_ids` | list of int    |          | `[]`        | WMO originating centre IDs (98 = ECMWF, 0 = WMO) |
| `access_ids` | list of string |          | `[]`        | Access tags (e.g. `"dissemination"`, `"research"`) |

Extra fields are allowed and are preserved in `get_metadata()` output.

The following raw API spellings are also accepted and normalised automatically:
`shortName`, `short_name`, `longName`, `long_name`, `name`.

**Avoid ID collisions** with the official ECMWF database by using IDs above
`900000` for your own parameters.

### Minimal example

```yaml
# my_params.yaml
- id: 900001
  shortname: myvar
  longname: My Custom Variable

- id: 900002
  shortname: myflux
  longname: My Custom Surface Flux
  units: W m**-2
  origin_ids: [98]
  access_ids: [research]
```

A fully-annotated starter file is provided at
`share/metkit/custom_param_example.yaml`.

### Validating entries against the schema

Use `ParameterEntry` (a Pydantic v2 model) to validate your YAML before loading:

```python
import pydantic
import yaml
from pymetkit import ParameterEntry

entries = yaml.safe_load(open("my_params.yaml"))

for raw in entries:
    try:
        ParameterEntry.model_validate(raw)
    except pydantic.ValidationError as exc:
        print(f"Invalid entry (id={raw.get('id')}): {exc}")
```

`ParameterEntry.model_validate` raises `pydantic.ValidationError` if:
- `id`, `shortname`, or `longname` is missing or empty
- `id` cannot be coerced to an integer
- `origin_ids` contains non-integer values

Valid entries are coerced silently (e.g. a string `"130"` for `id` becomes
`130`, `None` for `units` becomes `"unknown"`).

### Machine-readable JSON schema

A JSON Schema file is published at `share/metkit/parameter_entry_schema.json`.
You can use it with any JSON Schema–compliant validator:

```python
import json
import jsonschema
import yaml

schema  = json.load(open("share/metkit/parameter_entry_schema.json"))
entries = yaml.safe_load(open("my_params.yaml"))

for entry in entries:
    jsonschema.validate(instance=entry, schema=schema)
```

The schema file is regenerated automatically when you run
`generate_parameter_metadata.py`.

VS Code users: add a `# yaml-language-server: $schema=...` comment at the top
of your YAML file to get inline validation and autocompletion:

```yaml
# yaml-language-server: $schema=../../share/metkit/parameter_entry_schema.json
- id: 900001
  shortname: myvar
  longname: My Custom Variable
```

---

## Regenerating bundled data files (release process)

The bundled parameter metadata must be regenerated before each release to pick
up any changes from the ECMWF parameter database API. Run:

```bash
python -m pymetkit.generate_parameter_metadata
```

This produces the following files in `share/metkit/`:

| File                           | Purpose                                          |
|--------------------------------|--------------------------------------------------|
| `parameter_metadata.yaml`      | Human-readable parameter database (8,200+ entries) |
| `parameter_metadata.json`      | Fast-load format (identical data, ~200× faster to parse) |
| `unit_metadata.yaml`           | Unit definitions (208 entries)                   |
| `parameter_entry_schema.json`  | JSON Schema for validating custom YAML files     |

The script requires network access to `codes.ecmwf.int` and typically takes
2–3 minutes (it queries each originating centre individually to build the
`origin_ids` mapping).

Symlinks in the package source directory (`python/pymetkit/src/pymetkit/`) point
to `share/metkit/` so that the data files are included in the built wheel via
`importlib.resources`.
