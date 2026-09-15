# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

import argparse
import json
import logging
import os
import sys
from pathlib import Path

import findlibs

import pymetkit._internal as _internal

# INFO: This is in place because we currently can't (at runtime)
# tell which order the dependencies are in. This needs to be available in findlibs
DEPENDENCY_ORDER = ["eckit", "eccodes", "metkit"]
OPTIONAL_DEPENDENCIES = ["eccodes"]

# src/pymetkit/__main__.py -> src/ -> <repo_root>
_DEFAULT_METADATA_DIR = Path(__file__).parents[2] / "share" / "metkit"


def _generate_metadata(output_dir: Path) -> None:
    """Fetch parameter metadata from the ECMWF API and write it to output_dir."""
    import pymetkit.paramdb.generate_metadata as _gen
    from pymetkit.paramdb.models import MarsRequestContext, ParameterEntry

    output_dir.mkdir(parents=True, exist_ok=True)

    units, unit_map = _gen.fetch_units()
    _gen.write_unit_yaml(units, output_dir / "unit_metadata.yaml")

    _, param_origin_map = _gen.fetch_origin_map()
    parameters = _gen.fetch_parameters(unit_map=unit_map, param_origin_map=param_origin_map)

    context_map = _gen.build_param_context_map(_gen.LANGUAGE_PARAMS_YAML)
    _gen.enrich_parameters(parameters, context_map)

    _gen.write_param_yaml(parameters, output_dir / "parameter_metadata.yaml")
    _gen.write_param_json(parameters, output_dir / "parameter_metadata.json")

    schema = ParameterEntry.model_json_schema()
    (output_dir / "parameter_entry_schema.json").write_text(
        json.dumps(schema, indent=2), encoding="utf-8"
    )
    print(f"Written JSON schema to {output_dir / 'parameter_entry_schema.json'}")

    ctx_schema = MarsRequestContext.model_json_schema()
    (output_dir / "mars_context_schema.json").write_text(
        json.dumps(ctx_schema, indent=2), encoding="utf-8"
    )
    print(f"Written MARS context schema to {output_dir / 'mars_context_schema.json'}")


def main():
    parser = argparse.ArgumentParser(description="pymetkit command line interface")
    parser.add_argument(
        "--print-home",
        action="store_true",
        help="Print the home directory of the metkit library",
    )
    parser.add_argument(
        "--print-home-deps",
        action="store_true",
        help="Print the home directories of all pymetkit dependencies",
    )

    meta_group = parser.add_argument_group("metadata generation")
    meta_group.add_argument(
        "--generate-metadata",
        action="store_true",
        help="Fetch parameter metadata from the ECMWF API and write bundled data files",
    )
    meta_group.add_argument(
        "--metadata-dir",
        type=Path,
        default=_DEFAULT_METADATA_DIR,
        metavar="DIR",
        help=f"Output directory for --generate-metadata (default: {_DEFAULT_METADATA_DIR})",
    )

    parser.add_argument(
        "-v",
        "--verbose",
        action="store_true",
        help="Enable DEBUG logging (default: INFO)",
    )
    args = parser.parse_args()

    logging.basicConfig(
        format="%(asctime)s | %(levelname)-6s | %(message)s",
        datefmt="%Y-%m-%d %H:%M:%S",
        level=logging.DEBUG if args.verbose else logging.INFO,
    )

    if not (args.print_home or args.print_home_deps or args.generate_metadata):
        parser.print_help()
        sys.exit(2)

    if args.generate_metadata:
        _generate_metadata(args.metadata_dir)
        return

    def _lib_home(lib_path):
        lib_dir = os.path.dirname(os.path.realpath(lib_path))
        return (
            os.path.dirname(lib_dir)
            if os.path.basename(lib_dir) in ("lib", "lib64")
            else lib_dir
        )

    def _print_dep_path(lib, dependency_path, optional):
        missing = []
        if dependency_path is None:
            msg = f"\t{lib} [Optional]" if optional else f"\t{lib}"
            msg += ": not found by findlibs"
            missing.append(lib)
            if optional:
                logging.info(msg)
            else:
                logging.error(msg)
        else:
            msg = f"\t{lib} [Optional]" if optional else f"\t{lib}"
            msg += f": {_lib_home(dependency_path)}"
            logging.info(msg)

        return missing

    library_info_tuple = _internal.version_info()

    if args.print_home:
        dependency_path = findlibs.find("metkit")
        if dependency_path is None:
            logging.error("metkit library not found by findlibs")
            sys.exit(1)
        for name, version, gitSha, path in library_info_tuple:
            if name == "metkit":
                logging.info(f"\t{name} {version} ({gitSha}) {path}")

    if args.print_home_deps:
        logging.info("Findlibs Environment:")
        for key, value in os.environ.items():
            if key.upper().startswith("FINDLIBS_DISABLE"):
                logging.info(f"\t{key: <15}: {value: <10}")

        logging.info("Findlibs Lookup")

        missing = []
        for lib in DEPENDENCY_ORDER:
            dependency_path = findlibs.find(lib)
            missing += _print_dep_path(
                lib, dependency_path, lib in OPTIONAL_DEPENDENCIES
            )

        logging.info("Dependency Versions:")

        for name, version, gitSha, path in library_info_tuple:
            logging.info(f"\t{name} {version} ({gitSha}) {path}")

        if any(lib not in OPTIONAL_DEPENDENCIES for lib in missing):
            sys.exit(1)


if __name__ == "__main__":
    main()
