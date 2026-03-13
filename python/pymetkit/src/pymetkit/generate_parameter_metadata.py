"""
Standalone script to generate parameter_metadata.yaml by fetching all
parameters from the ECMWF parameter database API.

Usage
-----
    python -m pymetkit.generate_parameter_metadata
    # or directly:
    python generate_parameter_metadata.py
"""

import requests
import yaml
from pathlib import Path

API_URL = "https://codes.ecmwf.int/parameter-database/api/v1/param/"
OUTPUT_PATH = Path(__file__).parent / "parameter_metadata.yaml"


def _normalise(raw: dict) -> dict:
    """Return a normalised parameter dict with canonical key names."""
    entry = dict(raw)
    for key in ("shortName", "short_name"):
        if key in entry:
            entry["shortname"] = entry.pop(key)
            break
    if "id" in entry:
        entry["id"] = int(entry["id"])
    return entry


def fetch_parameters(url: str = API_URL) -> list[dict]:
    """Fetch all parameters from the ECMWF parameter database API."""
    print(f"Fetching parameters from {url} ...")
    response = requests.get(url)
    response.raise_for_status()
    params = response.json()
    print(f"  Received {len(params)} parameters.")
    return [_normalise(p) for p in params]


def write_yaml(params: list[dict], output_path: Path = OUTPUT_PATH) -> None:
    """Write the parameter list to a YAML file."""
    with output_path.open("w") as fh:
        yaml.dump(params, fh, default_flow_style=False, allow_unicode=True)
    print(f"Written {len(params)} parameters to {output_path}")


if __name__ == "__main__":
    parameters = fetch_parameters()
    write_yaml(parameters)
