"""
Standalone script to generate:
  - parameter_metadata.yaml  — one entry per ECMWF parameter
  - unit_metadata.yaml       — one entry per ECMWF unit

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
UNIT_OUTPUT = _REPO_ROOT / "share" / "metkit" / "unit_metadata.yaml"
SCHEMA_OUTPUT = _REPO_ROOT / "share" / "metkit" / "parameter_entry_schema.json"

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


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    units, unit_map = fetch_units()
    write_unit_yaml(units)

    _, param_origin_map = fetch_origin_map()

    parameters = fetch_parameters(unit_map=unit_map, param_origin_map=param_origin_map)
    write_param_yaml(parameters)

    # Write the JSON schema for ParameterEntry so downstream tools can validate YAML.
    from .models import ParameterEntry  # noqa: E402 (local import to avoid circular at module level)

    schema = ParameterEntry.model_json_schema()
    SCHEMA_OUTPUT.write_text(json.dumps(schema, indent=2), encoding="utf-8")
    print(f"Written JSON schema to {SCHEMA_OUTPUT}")
