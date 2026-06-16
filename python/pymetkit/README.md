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

db.shortname_to_param_id("t")     # → 130
db.param_id_to_shortname(130)     # → "t"
db.shortname_to_longname("t")     # → "Temperature"
db.param_id_to_longname(130)      # → "Temperature"
db.get_units(130)                  # → "K"
db.get_metadata(130)               # → full metadata dict
```

### Collision resolution

Some shortnames appear in more than one GRIB table or originating centre.
Pass `table=`, `origin=`, or `access=` to disambiguate:

```python
# Default: prefers dissemination params → ECMWF origin → lowest id
db.shortname_to_param_id("tp")                    # → 228

# Explicit table override
db.shortname_to_param_id("tp", table=228)         # → 228228

# Explicit origin (98 = ECMWF)
db.shortname_to_param_id("t", origin=98)          # → 130

# Access filter
db.shortname_to_param_id("tp", access="dissemination")  # → 228

# Inspect all candidates for a colliding shortname
db.get_all_by_shortname("tp")
# [{'id': 228, 'shortname': 'tp', ...}, {'id': 228228, 'shortname': 'tp', ...}]

db.shortname_has_collisions("tp")  # → True
db.shortname_has_collisions("t")   # → False (only one candidate)
```

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
