# SPDX-FileCopyrightText: 2026 European Centre for Medium-Range Weather Forecasts (ECMWF)
# SPDX-License-Identifier: Apache-2.0

import logging
import sys
from pathlib import Path
from unittest.mock import MagicMock

import findlibs
import pymetkit._internal as _internal
import pytest

from pymetkit.__main__ import DEPENDENCY_ORDER, OPTIONAL_DEPENDENCIES, main

ALL_LIBS = {
    "eckit": "/fake/eckit/lib/libeckit.so",
    "eccodes": "/fake/eccodes/lib/libeccodes.so",
    "metkit": "/fake/metkit/lib/libmetkit.so",
}

FAKE_VERSION_INFO = [
    ("eckit", "1.32.5", "def5678", "/fake/eckit/lib/libeckit.so"),
    ("eccodes", "2.46.0", "jkl3456", "/fake/eccodes/lib/libeccodes.so"),
    ("metkit", "1.19.0", "ghi9012", "/fake/metkit/lib/libmetkit.so"),
]


def _run_cli(args, monkeypatch):
    monkeypatch.setattr(sys, "argv", ["pymetkit"] + args)
    try:
        main()
        return 0
    except SystemExit as exc:
        return exc.code


def _expected_entry(name, path):
    """Build the expected log fragment for a findlibs lookup line."""
    label = f"{name} [Optional]" if name in OPTIONAL_DEPENDENCIES else name
    home = Path(path).parent.parent
    return f"{label}: {home}"


@pytest.fixture(autouse=True)
def capture_info_logs(caplog):
    caplog.set_level(logging.INFO)


@pytest.fixture(autouse=True)
def version_info_mock(monkeypatch):
    mock = MagicMock(return_value=FAKE_VERSION_INFO)
    monkeypatch.setattr(_internal, "version_info", mock)
    return mock


@pytest.fixture
def find_mock(monkeypatch):
    mock = MagicMock(side_effect=lambda name: ALL_LIBS.get(name))
    monkeypatch.setattr(findlibs, "find", mock)
    return mock


# ---------------------------------------------------------------------------
# --print-home
# ---------------------------------------------------------------------------


def test_print_home_success(find_mock, monkeypatch, caplog):
    exit_code = _run_cli(["--print-home"], monkeypatch)
    assert exit_code == 0


def test_print_home_logs_version_info(find_mock, monkeypatch, caplog):
    _run_cli(["--print-home"], monkeypatch)
    name, version, git_sha, path = FAKE_VERSION_INFO[-1]  # metkit entry
    assert name in caplog.text
    assert version in caplog.text
    assert git_sha in caplog.text


def test_print_home_not_found(monkeypatch, caplog):
    monkeypatch.setattr(findlibs, "find", MagicMock(return_value=None))
    exit_code = _run_cli(["--print-home"], monkeypatch)
    assert exit_code == 1
    assert "not found by findlibs" in caplog.text


def test_print_home_calls_find_with_metkit(find_mock, monkeypatch):
    _run_cli(["--print-home"], monkeypatch)
    find_mock.assert_called_once_with("metkit")


# ---------------------------------------------------------------------------
# --print-home-deps
# ---------------------------------------------------------------------------


def test_print_home_deps_all_found(find_mock, monkeypatch, caplog):
    exit_code = _run_cli(["--print-home-deps"], monkeypatch)
    assert exit_code == 0
    for name, path in ALL_LIBS.items():
        assert _expected_entry(name, path) in caplog.text


def test_print_home_deps_logs_dependency_versions(find_mock, monkeypatch, caplog):
    _run_cli(["--print-home-deps"], monkeypatch)
    assert "Dependency Versions:" in caplog.text
    for name, version, git_sha, _ in FAKE_VERSION_INFO:
        assert name in caplog.text
        assert version in caplog.text
        assert git_sha in caplog.text


def test_print_home_deps_dependency_versions_logged_before_exit(monkeypatch, caplog):
    libs_without_eckit = {k: v for k, v in ALL_LIBS.items() if k != "eckit"}
    monkeypatch.setattr(
        findlibs, "find", MagicMock(side_effect=lambda n: libs_without_eckit.get(n))
    )
    _run_cli(["--print-home-deps"], monkeypatch)
    assert "Dependency Versions:" in caplog.text
    for name, version, git_sha, _ in FAKE_VERSION_INFO:
        assert name in caplog.text
        assert version in caplog.text
        assert git_sha in caplog.text


def test_print_home_deps_missing_required_exits_nonzero(monkeypatch, caplog):
    libs_without_eckit = {k: v for k, v in ALL_LIBS.items() if k != "eckit"}
    monkeypatch.setattr(
        findlibs, "find", MagicMock(side_effect=lambda n: libs_without_eckit.get(n))
    )
    exit_code = _run_cli(["--print-home-deps"], monkeypatch)
    assert exit_code == 1
    assert "eckit" in caplog.text


def test_print_home_deps_missing_required_logs_error(monkeypatch, caplog):
    libs_without_eckit = {k: v for k, v in ALL_LIBS.items() if k != "eckit"}
    monkeypatch.setattr(
        findlibs, "find", MagicMock(side_effect=lambda n: libs_without_eckit.get(n))
    )
    _run_cli(["--print-home-deps"], monkeypatch)
    errors = [
        r for r in caplog.records
        if r.levelno == logging.ERROR and "eckit" in r.message
    ]
    assert errors


def test_print_home_deps_missing_optional_exits_zero(monkeypatch):
    libs_without_eccodes = {k: v for k, v in ALL_LIBS.items() if k != "eccodes"}
    monkeypatch.setattr(
        findlibs, "find", MagicMock(side_effect=lambda n: libs_without_eccodes.get(n))
    )
    exit_code = _run_cli(["--print-home-deps"], monkeypatch)
    assert exit_code == 0


def test_print_home_deps_missing_optional_logs_info_not_error(monkeypatch, caplog):
    libs_without_eccodes = {k: v for k, v in ALL_LIBS.items() if k != "eccodes"}
    monkeypatch.setattr(
        findlibs, "find", MagicMock(side_effect=lambda n: libs_without_eccodes.get(n))
    )
    _run_cli(["--print-home-deps"], monkeypatch)
    eccodes_records = [r for r in caplog.records if "eccodes" in r.message]
    assert eccodes_records
    assert all(r.levelno == logging.INFO for r in eccodes_records)


def test_print_home_deps_optional_marker_in_message(monkeypatch, caplog):
    libs_without_eccodes = {k: v for k, v in ALL_LIBS.items() if k != "eccodes"}
    monkeypatch.setattr(
        findlibs, "find", MagicMock(side_effect=lambda n: libs_without_eccodes.get(n))
    )
    _run_cli(["--print-home-deps"], monkeypatch)
    assert "[Optional]" in caplog.text


def test_print_home_deps_queries_all_deps(find_mock, monkeypatch):
    _run_cli(["--print-home-deps"], monkeypatch)
    queried = {call.args[0] for call in find_mock.call_args_list}
    assert queried == set(DEPENDENCY_ORDER)


def test_print_home_deps_disable_vars_appear_before_homes(
    find_mock, monkeypatch, caplog
):
    monkeypatch.setenv("FINDLIBS_DISABLE_METKIT", "1")
    _run_cli(["--print-home-deps"], monkeypatch)
    lines = caplog.text.splitlines()
    disable_idx = next(
        i for i, line in enumerate(lines) if "FINDLIBS_DISABLE_METKIT" in line
    )
    first_dep_idx = next(i for i, line in enumerate(lines) if "metkit:" in line)
    assert disable_idx < first_dep_idx


# ---------------------------------------------------------------------------
# Output format
# ---------------------------------------------------------------------------


def test_logging_format(monkeypatch):
    calls = []
    monkeypatch.setattr(logging, "basicConfig", lambda **kwargs: calls.append(kwargs))
    monkeypatch.setattr(
        findlibs, "find", MagicMock(return_value="/fake/metkit/lib/libmetkit.so")
    )
    _run_cli(["--print-home"], monkeypatch)
    assert calls, "basicConfig must be called"
    fmt = calls[0]["format"]
    assert "%(asctime)s" in fmt
    assert "%(levelname)" in fmt
    assert "%(message)s" in fmt


def test_verbose_sets_debug_level(monkeypatch):
    calls = []
    monkeypatch.setattr(logging, "basicConfig", lambda **kwargs: calls.append(kwargs))
    monkeypatch.setattr(
        findlibs, "find", MagicMock(return_value="/fake/metkit/lib/libmetkit.so")
    )
    _run_cli(["--print-home", "--verbose"], monkeypatch)
    assert calls[0]["level"] == logging.DEBUG


def test_default_level_is_info(monkeypatch):
    calls = []
    monkeypatch.setattr(logging, "basicConfig", lambda **kwargs: calls.append(kwargs))
    monkeypatch.setattr(
        findlibs, "find", MagicMock(return_value="/fake/metkit/lib/libmetkit.so")
    )
    _run_cli(["--print-home"], monkeypatch)
    assert calls[0]["level"] == logging.INFO


# ---------------------------------------------------------------------------
# No arguments
# ---------------------------------------------------------------------------


def test_no_args_exits_with_code_2(monkeypatch):
    exit_code = _run_cli([], monkeypatch)
    assert exit_code == 2
