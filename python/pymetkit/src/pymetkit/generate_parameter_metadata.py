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

import requests
import yaml
from pathlib import Path

PARAM_URL = "https://codes.ecmwf.int/parameter-database/api/v1/param/"
UNIT_URL = "https://codes.ecmwf.int/parameter-database/api/v1/unit/"
PARAM_OUTPUT = Path(__file__).parent / "parameter_metadata.yaml"
UNIT_OUTPUT = Path(__file__).parent / "unit_metadata.yaml"


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
    response = requests.get(url)
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
# Parameters
# ---------------------------------------------------------------------------


def fetch_parameters(
    url: str = PARAM_URL, unit_map: dict[int, str] = None
) -> list[dict]:
    """Fetch all parameters from the ECMWF parameter database API."""
    print(f"Fetching parameters from {url} ...")
    response = requests.get(url)
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

        entry = {
            "id": int(raw["id"]),
            "shortname": shortname,
            "longname": raw.get("name", ""),
            "units": units,
            "description": raw.get("description", ""),
        }
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

    parameters = fetch_parameters(unit_map=unit_map)
    write_param_yaml(parameters)
