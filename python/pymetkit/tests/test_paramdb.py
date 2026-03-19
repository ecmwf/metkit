from contextlib import nullcontext as does_not_raise
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
