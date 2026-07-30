"""
Standalone script to generate:
  - parameter_metadata.yaml  — one entry per ECMWF parameter (human-readable)
  - parameter_metadata.json  — same data, compact JSON (fast-load, ~200× faster)
  - unit_metadata.yaml       — one entry per ECMWF unit
  - parameter_entry_schema.json — JSON Schema for ParameterEntry validation

Usage
-----
    python -m pymetkit.generate_parameter_metadata
    # or directly:
    python generate_parameter_metadata.py
"""

import json
import requests
import yaml
from pathlib import Path

PARAM_URL = "https://codes.ecmwf.int/parameter-database/api/v1/param/"
UNIT_URL = "https://codes.ecmwf.int/parameter-database/api/v1/unit/"
ORIGIN_URL = "https://codes.ecmwf.int/parameter-database/api/v1/origin/"

# Output paths: canonical location is share/metkit/ at the repo root, which is
# four parent directories above this module file:
#   python/pymetkit/src/pymetkit/ -> python/pymetkit/src/ -> python/pymetkit/
#   -> python/ -> <repo_root>
_REPO_ROOT = Path(__file__).parents[4]
PARAM_OUTPUT = _REPO_ROOT / "share" / "metkit" / "parameter_metadata.yaml"
PARAM_JSON_OUTPUT = _REPO_ROOT / "share" / "metkit" / "parameter_metadata.json"
UNIT_OUTPUT = _REPO_ROOT / "share" / "metkit" / "unit_metadata.yaml"
SCHEMA_OUTPUT = _REPO_ROOT / "share" / "metkit" / "parameter_entry_schema.json"
MARS_CONTEXT_SCHEMA_OUTPUT = (
    _REPO_ROOT / "share" / "metkit" / "mars_context_schema.json"
)

#: Authoritative MARS context->paramid map used by the C++ resolution engine.
#: Each entry is ``[{class, stream, type, levtype}, [paramid, ...]]``.
LANGUAGE_PARAMS_YAML = _REPO_ROOT / "share" / "metkit" / "params.yaml"

#: Timeout in seconds for HTTP requests to the ECMWF parameter database API.
REQUEST_TIMEOUT = 30


# ---------------------------------------------------------------------------
# Units
# ---------------------------------------------------------------------------


def fetch_units(url: str = UNIT_URL) -> tuple[list[dict], dict[int, str]]:
    """
    Fetch all units from the ECMWF parameter database API.

    Returns
    -------
    units : list[dict]
        Normalised unit records ready to be written to unit_metadata.yaml.
    unit_map : dict[int, str]
        Mapping of unit id -> unit name string for use in parameter enrichment.
    """
    print(f"Fetching units from {url} ...")
    response = requests.get(url, timeout=REQUEST_TIMEOUT)
    response.raise_for_status()
    raw_units = response.json()
    print(f"  Received {len(raw_units)} units.")

    units = []
    unit_map: dict[int, str] = {}

    for raw in raw_units:
        uid = int(raw["id"])
        # The API may use 'name', 'symbol', or 'label' for the unit string
        name = raw.get("name") or raw.get("symbol") or raw.get("label") or ""

        entry = {"id": uid}
        # Preserve all fields the API returns, but ensure id comes first
        for key, value in raw.items():
            if key == "id":
                continue
            entry[key] = value
        # Always emit a canonical 'name' field so unit_metadata.yaml has a
        # stable schema regardless of which key the API uses (name/symbol/label)
        entry["name"] = name

        units.append(entry)
        unit_map[uid] = name

    units.sort(key=lambda e: e["id"])
    return units, unit_map


def write_unit_yaml(units: list[dict], output_path: Path = UNIT_OUTPUT) -> None:
    """Write the unit list to a YAML file."""
    with output_path.open("w") as fh:
        yaml.dump(
            units,
            fh,
            default_flow_style=False,
            allow_unicode=True,
            sort_keys=False,
        )
    print(f"Written {len(units)} units to {output_path}")


# ---------------------------------------------------------------------------
# Origins
# ---------------------------------------------------------------------------


def fetch_origin_map(
    origin_url: str = ORIGIN_URL,
    param_url: str = PARAM_URL,
) -> tuple[dict[int, dict], dict[int, list[int]]]:
    """Fetch all origins and build a reverse map of param_id -> [origin_ids].

    The ``/param/`` endpoint does not include an ``origin`` field in its
    response, so we derive the mapping by querying each origin's filtered
    parameter list via ``/param/?origin=<id>``.

    Returns
    -------
    origins : dict[int, dict]
        Mapping of origin_id -> origin metadata (id, abbreviation, name).
    param_origin_map : dict[int, list[int]]
        Mapping of param_id -> sorted list of origin_ids that include it.
    """
    print(f"Fetching origins from {origin_url} ...")
    response = requests.get(origin_url, timeout=REQUEST_TIMEOUT)
    response.raise_for_status()
    raw_origins = response.json()
    print(f"  Received {len(raw_origins)} origins.")

    origins: dict[int, dict] = {o["id"]: o for o in raw_origins}
    param_origin_map: dict[int, list[int]] = {}

    for origin in raw_origins:
        oid = origin["id"]
        abbr = origin.get("abbreviation", str(oid))
        print(f"  Fetching params for origin={oid} ({abbr}) ...")
        r = requests.get(
            param_url, params={"origin": oid}, timeout=REQUEST_TIMEOUT
        )
        r.raise_for_status()
        origin_params = r.json()
        print(f"    {len(origin_params)} params.")
        for p in origin_params:
            pid = int(p["id"])
            param_origin_map.setdefault(pid, []).append(oid)

    # Sort each origin list for deterministic output
    for pid in param_origin_map:
        param_origin_map[pid].sort()

    return origins, param_origin_map


# ---------------------------------------------------------------------------
# Parameters
# ---------------------------------------------------------------------------


def fetch_parameters(
    url: str = PARAM_URL,
    unit_map: "dict[int, str] | None" = None,
    param_origin_map: "dict[int, list[int]] | None" = None,
) -> list[dict]:
    """Fetch all parameters from the ECMWF parameter database API.

    Parameters
    ----------
    url:
        The parameter API endpoint.
    unit_map:
        Mapping of unit_id -> unit name string, used to resolve the
        ``units`` field.  When ``None`` the units field is left empty.
    param_origin_map:
        Mapping of param_id -> list of origin_ids, built by
        :func:`fetch_origin_map`.  When provided, each entry gains an
        ``origin_ids`` field containing the sorted list of WMO originating
        centre IDs that include this parameter.  When ``None`` the field
        is omitted.
    """
    print(f"Fetching parameters from {url} ...")
    response = requests.get(url, timeout=REQUEST_TIMEOUT)
    response.raise_for_status()
    params = response.json()
    print(f"  Received {len(params)} parameters.")

    result = []
    for raw in params:
        # Resolve short name (API may return 'shortName', 'short_name', or 'shortname')
        shortname = (
            raw.get("shortname") or raw.get("shortName") or raw.get("short_name") or ""
        )

        # Resolve units via unit_map if available
        unit_id = raw.get("unit_id")
        if unit_map and unit_id is not None:
            units = unit_map.get(int(unit_id), "")
        else:
            units = ""

        pid = int(raw["id"])

        entry = {
            "id": pid,
            "shortname": shortname,
            "longname": raw.get("name", ""),
            "units": units,
            "description": raw.get("description", ""),
            # access_ids indicates dissemination availability; preserve as-is.
            "access_ids": raw.get("access_ids", []),
        }

        # Attach origin_ids derived from the per-origin filtered queries.
        if param_origin_map is not None:
            entry["origin_ids"] = param_origin_map.get(pid, [])

        result.append(entry)

    result.sort(key=lambda e: e["id"])
    return result


def write_param_yaml(params: list[dict], output_path: Path = PARAM_OUTPUT) -> None:
    """Write the parameter list to a YAML file."""
    with output_path.open("w") as fh:
        yaml.dump(
            params,
            fh,
            default_flow_style=False,
            allow_unicode=True,
            sort_keys=False,
        )
    print(f"Written {len(params)} parameters to {output_path}")


def write_param_json(
    params: list[dict], output_path: Path = PARAM_JSON_OUTPUT
) -> None:
    """Write the parameter list to a JSON file (fast-load format).

    This is functionally identical to the YAML but loads ~10-50× faster
    via ``json.load()`` compared to ``yaml.safe_load()``.
    """
    with output_path.open("w") as fh:
        json.dump(params, fh, ensure_ascii=False, separators=(",", ":"))
    print(f"Written {len(params)} parameters to {output_path}")


# ---------------------------------------------------------------------------
# Table + MARS context enrichment (derived locally — no network required)
# ---------------------------------------------------------------------------

#: Keys of a MARS context matcher in params.yaml, in canonical order.
_CONTEXT_KEYS = ("class", "stream", "type", "levtype")


def table_from_id(param_id: int) -> int:
    """Return the GRIB parameter table a *param_id* encodes to.

    Mirrors the C++ ``Param`` encoding / ``ParamDB._table_from_id``:
      * ``< 1000``      -> table 128 (classic ECMWF; prefix suppressed)
      * ``< 1_000_000`` -> ``param_id // 1000``
      * ``>= 1_000_000``-> ``(param_id % 1_000_000) // 1000``
    """
    if param_id < 1000:
        return 128
    if param_id < 1_000_000:
        return param_id // 1000
    return (param_id % 1_000_000) // 1000


def build_param_context_map(
    params_yaml_path: Path = LANGUAGE_PARAMS_YAML,
) -> dict[int, list[dict]]:
    """Invert ``params.yaml`` into ``param_id -> [context dict, ...]``.

    ``params.yaml`` lists, for each MARS context matcher
    (``{class, stream, type, levtype}``), the paramids valid in that context.
    This inverts it so each paramid maps to the list of contexts in which it
    appears — the raw material for disambiguating shortname collisions. The
    *minimal distinguishing* subset among a shortname's candidates is computed
    at query time, not here.
    """
    print(f"Reading MARS contexts from {params_yaml_path} ...")
    with params_yaml_path.open("r") as fh:
        rules = yaml.safe_load(fh)

    context_map: dict[int, list[dict]] = {}
    for matcher, ids in rules:
        # Canonicalise key order and drop absent keys.
        context = {k: matcher[k] for k in _CONTEXT_KEYS if k in matcher}
        for pid in ids:
            bucket = context_map.setdefault(int(pid), [])
            if context not in bucket:
                bucket.append(context)

    # Deterministic ordering of contexts per paramid.
    for pid in context_map:
        context_map[pid].sort(key=lambda c: tuple(c.get(k, "") for k in _CONTEXT_KEYS))
    print(f"  Mapped MARS contexts for {len(context_map)} paramids.")
    return context_map


def enrich_parameters(
    params: list[dict],
    context_map: "dict[int, list[dict]] | None" = None,
) -> list[dict]:
    """Add ``table`` and ``mars_request_context`` to each parameter entry.

    ``table`` is derived from the id encoding; ``mars_request_context`` is
    looked up from *context_map* (built by :func:`build_param_context_map`).
    Both are fully offline derivations — no network access required.
    """
    if context_map is None:
        context_map = build_param_context_map()

    for entry in params:
        pid = int(entry["id"])
        entry["table"] = table_from_id(pid)
        entry["mars_request_context"] = context_map.get(pid, [])
    return params


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    units, unit_map = fetch_units()
    write_unit_yaml(units)

    _, param_origin_map = fetch_origin_map()

    parameters = fetch_parameters(unit_map=unit_map, param_origin_map=param_origin_map)

    # Enrich with locally-derived table + MARS context (no network needed).
    context_map = build_param_context_map()
    enrich_parameters(parameters, context_map)

    write_param_yaml(parameters)
    write_param_json(parameters)

    # Write the JSON schemas so downstream tools can validate YAML.
    # The MARS context schema is kept SEPARATE from the parameter schema so the
    # context contract can evolve independently (and back user-supplied schemas).
    from .models import (  # noqa: E402 (local import to avoid circular at module level)
        MarsRequestContext,
        ParameterEntry,
    )

    schema = ParameterEntry.model_json_schema()
    SCHEMA_OUTPUT.write_text(json.dumps(schema, indent=2), encoding="utf-8")
    print(f"Written JSON schema to {SCHEMA_OUTPUT}")

    ctx_schema = MarsRequestContext.model_json_schema()
    MARS_CONTEXT_SCHEMA_OUTPUT.write_text(
        json.dumps(ctx_schema, indent=2), encoding="utf-8"
    )
    print(f"Written MARS context schema to {MARS_CONTEXT_SCHEMA_OUTPUT}")
