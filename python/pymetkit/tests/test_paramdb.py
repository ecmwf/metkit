from contextlib import nullcontext as does_not_raise
import json
from datetime import datetime, timedelta, timezone
from unittest.mock import MagicMock, patch
import pytest

from pymetkit import ParamDB


# ---------------------------------------------------------------------------
# Fixtures
# ---------------------------------------------------------------------------


@pytest.fixture(scope="module")
def db():
    """Shared offline ParamDB instance (loaded once per test module)."""
    return ParamDB()


# ---------------------------------------------------------------------------
# Construction / mode validation
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "mode, expectation",
    [
        ["offline", does_not_raise()],
        ["online", does_not_raise()],  # network call; skipped below if no requests
        ["invalid", pytest.raises(ValueError)],
        ["OFFLINE", pytest.raises(ValueError)],
    ],
)
def test_constructor_mode_validation(mode, expectation):
    """Only 'online' and 'offline' are accepted mode values."""
    if mode == "online":
        pytest.importorskip("requests")
    with expectation:
        ParamDB(mode=mode)


def test_offline_default():
    """ParamDB() with no arguments loads in offline mode without error."""
    db = ParamDB()
    # Sanity-check that data was actually loaded
    assert len(db._by_id) > 0
    assert len(db._by_shortname) > 0
    assert len(db._by_longname) > 0


def test_online_requires_requests(monkeypatch):
    """Online mode raises ImportError when the requests package is absent."""
    import pymetkit.pymetkit as _mod

    monkeypatch.setattr(_mod, "_requests", None)
    with pytest.raises(ImportError, match="requests"):
        ParamDB(mode="online")


# ---------------------------------------------------------------------------
# _normalise (static helper)
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "raw, expected_shortname, expected_longname",
    [
        # Already canonical keys
        [
            {"id": 1, "shortname": "strf", "longname": "Stream function"},
            "strf",
            "Stream function",
        ],
        # CamelCase aliases
        [
            {"id": 1, "shortName": "strf", "longName": "Stream function"},
            "strf",
            "Stream function",
        ],
        # snake_case aliases
        [
            {"id": 1, "short_name": "strf", "long_name": "Stream function"},
            "strf",
            "Stream function",
        ],
        # 'name' as longname fallback
        [
            {"id": 1, "shortname": "strf", "name": "Stream function"},
            "strf",
            "Stream function",
        ],
    ],
)
def test_normalise_key_aliases(raw, expected_shortname, expected_longname):
    result = ParamDB._normalise(raw)
    assert result["shortname"] == expected_shortname
    assert result["longname"] == expected_longname


def test_normalise_coerces_id_to_int():
    result = ParamDB._normalise({"id": "42", "shortname": "foo", "longname": "Foo"})
    assert result["id"] == 42
    assert isinstance(result["id"], int)


def test_normalise_preserves_extra_fields():
    raw = {
        "id": 1,
        "shortname": "strf",
        "longname": "Stream function",
        "units": "m**2 s**-1",
    }
    result = ParamDB._normalise(raw)
    assert result["units"] == "m**2 s**-1"


# ---------------------------------------------------------------------------
# param_id_to_shortname / param_id_to_longname
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "param_id, expected_shortname",
    [
        [1, "strf"],
        [4, "eqpt"],
        [6, "ssfr"],
    ],
)
def test_param_id_to_shortname(db, param_id, expected_shortname):
    assert db.param_id_to_shortname(param_id) == expected_shortname


def test_param_id_to_shortname_unknown(db):
    with pytest.raises(KeyError):
        db.param_id_to_shortname(999_999_999)


@pytest.mark.parametrize(
    "param_id, expected_longname",
    [
        [1, "Stream function"],
        [4, "Equivalent potential temperature"],
        [6, "Soil sand fraction"],
    ],
)
def test_param_id_to_longname(db, param_id, expected_longname):
    assert db.param_id_to_longname(param_id) == expected_longname


def test_param_id_to_longname_unknown(db):
    with pytest.raises(KeyError):
        db.param_id_to_longname(999_999_999)


# ---------------------------------------------------------------------------
# shortname_to_longname / shortname_to_param_id
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "shortname, expected_longname",
    [
        ["strf", "Stream function"],
        ["eqpt", "Equivalent potential temperature"],
        ["ssfr", "Soil sand fraction"],
    ],
)
def test_shortname_to_longname(db, shortname, expected_longname):
    assert db.shortname_to_longname(shortname) == expected_longname


def test_shortname_to_longname_unknown(db):
    with pytest.raises(KeyError):
        db.shortname_to_longname("not_a_real_shortname_xyz")


@pytest.mark.parametrize(
    "shortname, expected_id",
    [
        ["strf", 1],
        ["eqpt", 4],
        ["ssfr", 6],
    ],
)
def test_shortname_to_param_id(db, shortname, expected_id):
    assert db.shortname_to_param_id(shortname) == expected_id


def test_shortname_to_param_id_unknown(db):
    with pytest.raises(KeyError):
        db.shortname_to_param_id("not_a_real_shortname_xyz")


# ---------------------------------------------------------------------------
# longname_to_shortname / longname_to_param_id
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "longname, expected_shortname",
    [
        # Use entries where the longname is unique in the DB (last-write-wins)
        ["Equivalent potential temperature", "eqpt"],
        ["Soil sand fraction", "ssfr"],
        ["Soil clay fraction", "scfr"],
    ],
)
def test_longname_to_shortname(db, longname, expected_shortname):
    assert db.longname_to_shortname(longname) == expected_shortname


def test_longname_to_shortname_unknown(db):
    with pytest.raises(KeyError):
        db.longname_to_shortname("Not A Real Long Name XYZ")


@pytest.mark.parametrize(
    "longname, expected_id",
    [
        ["Equivalent potential temperature", 4],
        ["Soil sand fraction", 6],
        ["Soil clay fraction", 7],
    ],
)
def test_longname_to_param_id(db, longname, expected_id):
    assert db.longname_to_param_id(longname) == expected_id


def test_longname_to_param_id_unknown(db):
    with pytest.raises(KeyError):
        db.longname_to_param_id("Not A Real Long Name XYZ")


# ---------------------------------------------------------------------------
# Roundtrip consistency
# ---------------------------------------------------------------------------


@pytest.mark.parametrize("param_id", [1, 4, 6])
def test_roundtrip_id_shortname(db, param_id):
    shortname = db.param_id_to_shortname(param_id)
    assert db.shortname_to_param_id(shortname) == param_id


@pytest.mark.parametrize("param_id", [4, 6, 7])
def test_roundtrip_id_longname(db, param_id):
    longname = db.param_id_to_longname(param_id)
    assert db.longname_to_param_id(longname) == param_id


# ---------------------------------------------------------------------------
# get_metadata
# ---------------------------------------------------------------------------


def test_get_metadata_by_int(db):
    meta = db.get_metadata(1)
    assert meta["id"] == 1
    assert meta["shortname"] == "strf"
    assert meta["longname"] == "Stream function"


def test_get_metadata_by_shortname(db):
    meta = db.get_metadata("strf")
    assert meta["id"] == 1


def test_get_metadata_by_longname(db):
    # Use a longname that is unique in the DB (no later entry overwrites it)
    meta = db.get_metadata("Equivalent potential temperature")
    assert meta["id"] == 4


def test_get_metadata_by_numeric_string(db):
    """A string that looks like an integer should resolve via param id."""
    meta = db.get_metadata("1")
    assert meta["id"] == 1
    assert meta["shortname"] == "strf"


def test_get_metadata_unknown_raises(db):
    with pytest.raises(KeyError):
        db.get_metadata(999_999_999)


def test_get_metadata_unknown_string_raises(db):
    with pytest.raises(KeyError):
        db.get_metadata("not_a_param_xyz")


def test_get_metadata_returns_dict(db):
    meta = db.get_metadata(1)
    assert isinstance(meta, dict)


# ---------------------------------------------------------------------------
# get_units
# ---------------------------------------------------------------------------


@pytest.mark.parametrize(
    "identifier, expected_units",
    [
        [1, "m**2 s**-1"],
        ["strf", "m**2 s**-1"],
        # Use a stable longname for the longname-based lookup
        ["Equivalent potential temperature", "K"],
        [6, "(0 - 1)"],
        ["ssfr", "(0 - 1)"],
    ],
)
def test_get_units(db, identifier, expected_units):
    assert db.get_units(identifier) == expected_units


def test_get_units_unknown_raises(db):
    with pytest.raises(KeyError):
        db.get_units(999_999_999)


def test_get_units_missing_returns_unknown():
    """When a parameter entry has no 'units' key, get_units returns 'unknown'."""
    db = ParamDB.__new__(ParamDB)
    db._by_id = {9999: {"id": 9999, "shortname": "foo", "longname": "Foo"}}
    db._by_shortname = {"foo": db._by_id[9999]}
    db._by_longname = {"Foo": db._by_id[9999]}
    assert db.get_units(9999) == "unknown"


def test_get_units_empty_string_returns_unknown():
    """When a parameter's units value is an empty string, get_units returns 'unknown'."""
    db = ParamDB.__new__(ParamDB)
    entry = {"id": 9998, "shortname": "bar", "longname": "Bar", "units": ""}
    db._by_id = {9998: entry}
    db._by_shortname = {"bar": entry}
    db._by_longname = {"Bar": entry}
    assert db.get_units(9998) == "unknown"


# ---------------------------------------------------------------------------
# Online caching
# ---------------------------------------------------------------------------

#: Minimal parameter list returned by a mocked API response.
_FAKE_PARAMS = [
    {
        "id": 1,
        "shortname": "strf",
        "longname": "Stream function",
        "units": "m**2 s**-1",
    },
    {
        "id": 4,
        "shortname": "eqpt",
        "longname": "Equivalent potential temperature",
        "units": "K",
    },
]


@pytest.fixture()
def fake_requests(monkeypatch):
    """Monkeypatch the requests module with a mock that returns _FAKE_PARAMS."""
    import pymetkit.pymetkit as _mod

    mock_resp = MagicMock()
    mock_resp.json.return_value = _FAKE_PARAMS
    mock_requests = MagicMock()
    mock_requests.get.return_value = mock_resp
    monkeypatch.setattr(_mod, "_requests", mock_requests)
    return mock_requests


def test_cache_ttl_must_be_timedelta(fake_requests, tmp_path):
    """Passing a non-timedelta cache_ttl raises TypeError."""
    with pytest.raises(TypeError, match="timedelta"):
        ParamDB(mode="online", cache_ttl=3600, cache_path=tmp_path)


def test_online_writes_cache_file(fake_requests, tmp_path):
    """First online load writes a JSON cache file to the cache directory."""
    ParamDB(mode="online", cache_path=tmp_path)
    cache_file = tmp_path / ParamDB._CACHE_FILENAME
    assert cache_file.exists()
    payload = json.loads(cache_file.read_text())
    assert "fetched_at" in payload
    assert "params" in payload
    assert payload["params"] == _FAKE_PARAMS


def test_online_uses_fresh_cache(fake_requests, tmp_path):
    """A second instantiation within the TTL window does not make an HTTP request."""
    ParamDB(mode="online", cache_path=tmp_path)
    assert fake_requests.get.call_count == 1

    ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(hours=1))
    assert fake_requests.get.call_count == 1  # no new request


def test_online_fetches_when_cache_expired(fake_requests, tmp_path):
    """An expired cache entry triggers a fresh HTTP request."""
    # Write a cache file whose timestamp is in the past
    old_time = datetime.now(tz=timezone.utc) - timedelta(hours=2)
    payload = {"fetched_at": old_time.isoformat(), "params": _FAKE_PARAMS}
    cache_file = tmp_path / ParamDB._CACHE_FILENAME
    cache_file.write_text(json.dumps(payload))

    ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(hours=1))
    assert fake_requests.get.call_count == 1  # stale cache → new request


def test_online_fetches_when_cache_not_yet_expired(fake_requests, tmp_path):
    """A cache entry within the TTL does NOT trigger a new request."""
    recent_time = datetime.now(tz=timezone.utc) - timedelta(minutes=30)
    payload = {"fetched_at": recent_time.isoformat(), "params": _FAKE_PARAMS}
    cache_file = tmp_path / ParamDB._CACHE_FILENAME
    cache_file.write_text(json.dumps(payload))

    ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(hours=1))
    assert fake_requests.get.call_count == 0  # fresh cache → no request


def test_online_zero_ttl_bypasses_cache(fake_requests, tmp_path):
    """cache_ttl=timedelta(0) disables caching: always fetches and never writes a file."""
    ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(0))
    assert fake_requests.get.call_count == 1
    assert not (tmp_path / ParamDB._CACHE_FILENAME).exists()

    ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(0))
    assert fake_requests.get.call_count == 2  # second call also fetches


def test_online_corrupt_cache_falls_back_to_fetch(fake_requests, tmp_path):
    """A corrupt/unreadable cache file is ignored and a fresh request is made."""
    cache_file = tmp_path / ParamDB._CACHE_FILENAME
    cache_file.write_text("this is not valid json {{{")

    ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(hours=1))
    assert fake_requests.get.call_count == 1


def test_online_cache_overwrites_after_expiry(fake_requests, tmp_path):
    """After a stale cache triggers a fetch, the cache file is updated."""
    old_time = datetime.now(tz=timezone.utc) - timedelta(hours=2)
    payload = {"fetched_at": old_time.isoformat(), "params": _FAKE_PARAMS}
    cache_file = tmp_path / ParamDB._CACHE_FILENAME
    cache_file.write_text(json.dumps(payload))

    ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(hours=1))

    updated = json.loads(cache_file.read_text())
    updated_time = datetime.fromisoformat(updated["fetched_at"])
    if updated_time.tzinfo is None:
        updated_time = updated_time.replace(tzinfo=timezone.utc)
    assert updated_time > old_time


def test_online_cache_data_is_usable(fake_requests, tmp_path):
    """Data loaded from cache round-trips correctly through _normalise and _index."""
    # Populate the cache
    ParamDB(mode="online", cache_path=tmp_path)

    # Load from cache (no network call)
    db = ParamDB(mode="online", cache_path=tmp_path, cache_ttl=timedelta(hours=1))
    assert fake_requests.get.call_count == 1  # only the first call used the network
    assert db.param_id_to_shortname(1) == "strf"
    assert db.param_id_to_shortname(4) == "eqpt"


def test_online_no_platformdirs_no_cache_dir(fake_requests, tmp_path, monkeypatch):
    """When platformdirs is unavailable and no cache_path is given, caching is
    silently skipped and the data is still loaded correctly."""
    import pymetkit.pymetkit as _mod

    monkeypatch.setattr(_mod, "_platformdirs", None)
    db = ParamDB(mode="online")  # no cache_path, no platformdirs
    assert fake_requests.get.call_count == 1
    assert db.param_id_to_shortname(1) == "strf"


def test_online_default_ttl_is_one_hour():
    """The documented default TTL is exactly one hour."""
    assert ParamDB._DEFAULT_CACHE_TTL == timedelta(hours=1)


# ---------------------------------------------------------------------------
# yaml_path parameter
# ---------------------------------------------------------------------------

#: Minimal YAML content used by the yaml_path tests.
_CUSTOM_YAML = """\
- id: 101
  shortname: cust1
  longname: Custom param one
  units: m s**-1
  description: First custom parameter
- id: 102
  shortname: cust2
  longname: Custom param two
  units: K
  description: Second custom parameter
"""

#: Same data with alternate key names to exercise _normalise.
_CUSTOM_YAML_ALIASES = """\
- id: 201
  shortName: alias1
  longName: Alias param one
  units: Pa
- id: 202
  short_name: alias2
  long_name: Alias param two
  units: kg m**-3
"""


@pytest.fixture()
def custom_yaml(tmp_path):
    """Write _CUSTOM_YAML to a temporary file and return its Path."""
    p = tmp_path / "custom_params.yaml"
    p.write_text(_CUSTOM_YAML)
    return p


@pytest.fixture()
def custom_yaml_aliases(tmp_path):
    """Write _CUSTOM_YAML_ALIASES to a temporary file and return its Path."""
    p = tmp_path / "alias_params.yaml"
    p.write_text(_CUSTOM_YAML_ALIASES)
    return p


def test_yaml_path_loads_custom_file(custom_yaml):
    """yaml_path pointing at a valid YAML file loads without error."""
    db = ParamDB(yaml_path=custom_yaml)
    assert db.param_id_to_shortname(101) == "cust1"
    assert db.param_id_to_shortname(102) == "cust2"


def test_yaml_path_as_string(custom_yaml):
    """yaml_path accepts a plain string as well as a Path object."""
    db = ParamDB(yaml_path=str(custom_yaml))
    assert db.param_id_to_shortname(101) == "cust1"


def test_yaml_path_data_is_used_not_default(custom_yaml):
    """Custom YAML is actually loaded; the standard DB entries are NOT present."""
    db = ParamDB(yaml_path=custom_yaml)
    # Custom entries exist
    assert db.param_id_to_shortname(101) == "cust1"
    # Standard entry is absent
    with pytest.raises(KeyError):
        db.param_id_to_shortname(1)


def test_yaml_path_full_lookup_chain(custom_yaml):
    """All three lookup indices work correctly for a custom YAML file."""
    db = ParamDB(yaml_path=custom_yaml)
    # id → shortname / longname
    assert db.param_id_to_shortname(101) == "cust1"
    assert db.param_id_to_longname(101) == "Custom param one"
    # shortname → id / longname
    assert db.shortname_to_param_id("cust1") == 101
    assert db.shortname_to_longname("cust1") == "Custom param one"
    # longname → id / shortname
    assert db.longname_to_param_id("Custom param one") == 101
    assert db.longname_to_shortname("Custom param one") == "cust1"


def test_yaml_path_get_metadata(custom_yaml):
    """get_metadata works for all identifier types with a custom YAML file."""
    db = ParamDB(yaml_path=custom_yaml)
    assert db.get_metadata(101)["shortname"] == "cust1"
    assert db.get_metadata("cust1")["id"] == 101
    assert db.get_metadata("Custom param one")["id"] == 101
    assert db.get_metadata("101")["shortname"] == "cust1"


def test_yaml_path_get_units(custom_yaml):
    """get_units returns the correct value from a custom YAML file."""
    db = ParamDB(yaml_path=custom_yaml)
    assert db.get_units(101) == "m s**-1"
    assert db.get_units(102) == "K"


def test_yaml_path_normalises_key_aliases(custom_yaml_aliases):
    """Alternate key names (shortName, longName, etc.) are normalised correctly."""
    db = ParamDB(yaml_path=custom_yaml_aliases)
    assert db.param_id_to_shortname(201) == "alias1"
    assert db.param_id_to_longname(201) == "Alias param one"
    assert db.param_id_to_shortname(202) == "alias2"
    assert db.param_id_to_longname(202) == "Alias param two"


def test_yaml_path_missing_file_raises(tmp_path):
    """A yaml_path that does not exist raises FileNotFoundError."""
    missing = tmp_path / "does_not_exist.yaml"
    with pytest.raises(FileNotFoundError):
        ParamDB(yaml_path=missing)


def test_yaml_path_with_online_mode_raises(custom_yaml):
    """Combining yaml_path with mode='online' raises ValueError."""
    with pytest.raises(ValueError, match="yaml_path"):
        ParamDB(mode="online", yaml_path=custom_yaml)


def test_yaml_path_none_loads_default_yaml():
    """yaml_path=None (the default) loads the standard parameter_metadata.yaml."""
    db = ParamDB(yaml_path=None)
    # The standard DB has many entries; a well-known entry should be present.
    assert db.param_id_to_shortname(1) == "strf"
    assert len(db._by_id) > 100
