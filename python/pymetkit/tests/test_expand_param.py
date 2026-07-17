"""Esoteric, context-dependent tests for ``expand_key("param", ...)``.

Unlike single-pass keys, ``param`` is resolved by a full multi-pass expansion:
its numeric id depends on the *other* keys in the request (``class``, ``stream``,
``type``, ``levtype``). ``expand_key`` hides that by building a scoped request
from the supplied ``context``, expanding it (``inherit=False`` -- only the given
context drives resolution), and returning the resolved ``param`` values.

These tests exercise the interesting corners: shortname->id lookup, the four
context axes that flip the result, the GRIB ``param.table`` numeric encoding,
multi-value lists, ``all``/``off`` handling, insufficient-context fallback, and
error cases.
"""

import pytest

from pymetkit import MarsRequest, MetKitException, expand_key


# Fully-specified operational contexts (all four matcher keys present).
OD_PL = {"class": "od", "stream": "oper", "type": "an", "levtype": "pl"}
OD_SFC = {"class": "od", "stream": "oper", "type": "an", "levtype": "sfc"}


def _param(value, context):
    return expand_key("param", value, context=context)


# ---------------------------------------------------------------------------
# Shortname -> paramid lookup on pressure levels
# ---------------------------------------------------------------------------
@pytest.mark.parametrize(
    "shortname, expected",
    [
        ("t", "130"),
        ("z", "129"),
        ("q", "133"),
        ("u", "131"),
        ("v", "132"),
        ("w", "135"),
        ("r", "157"),
        ("vo", "138"),
        ("d", "155"),
        ("gh", "156"),  # geopotential *height*, distinct from z (geopotential)
    ],
)
def test_pressure_level_shortnames(shortname, expected):
    assert _param(shortname, OD_PL) == [expected]


# ---------------------------------------------------------------------------
# Shortname -> paramid lookup on the surface
# ---------------------------------------------------------------------------
@pytest.mark.parametrize(
    "shortname, expected",
    [
        ("2t", "167"),
        ("2d", "168"),
        ("10u", "165"),
        ("10v", "166"),
        ("msl", "151"),
        ("sp", "134"),
        ("skt", "235"),
        ("lsm", "172"),
        ("z", "129"),
    ],
)
def test_surface_shortnames(shortname, expected):
    assert _param(shortname, OD_SFC) == [expected]


# ---------------------------------------------------------------------------
# The four context axes that flip the resolved id
# ---------------------------------------------------------------------------
def test_levtype_flips_result():
    # 't' is geopotential-level temperature (130) on pressure levels but
    # 2m temperature (164) on the operational surface.
    assert _param("t", OD_PL) == ["130"]
    assert _param("t", OD_SFC) == ["164"]


def test_class_flips_result():
    # Same surface 't', different class: only 'od' maps to the 2m id.
    assert _param("t", {**OD_SFC, "class": "od"}) == ["164"]
    assert _param("t", {**OD_SFC, "class": "ei"}) == ["130"]


def test_type_flips_result():
    # Same surface 't', different type: analysis vs forecast resolve differently.
    assert _param("t", {**OD_SFC, "type": "an"}) == ["164"]
    assert _param("t", {**OD_SFC, "type": "fc"}) == ["130"]


def test_stream_flips_result():
    # 'swh' (significant wave height) resolves differently in the wave stream.
    assert _param("swh", {**OD_SFC, "stream": "wave"}) == ["140229"]
    assert _param("swh", {**OD_SFC, "stream": "oper"}) == ["3100"]


# ---------------------------------------------------------------------------
# Numeric ids pass through unchanged (still validated against the context)
# ---------------------------------------------------------------------------
@pytest.mark.parametrize("value", ["130", "129", "62", "260048"])
def test_numeric_passthrough(value):
    assert _param(value, OD_SFC) == [value]


# ---------------------------------------------------------------------------
# GRIB 'param.table' encoding: 'PP.TT' -> str(TT * 1000 + PP), table 128 -> 0
# ---------------------------------------------------------------------------
@pytest.mark.parametrize(
    "encoded, expected",
    [
        ("151.128", "151"),      # table 128 collapses to 0 -> bare param
        ("228.128", "228"),
        ("228.228", "228228"),   # 228*1000 + 228
        ("129.228", "228129"),   # 228*1000 + 129
        ("8.171", "171008"),     # 171*1000 + 8, zero-padded param
    ],
)
def test_param_table_encoding(encoded, expected):
    assert _param(encoded, OD_SFC) == [expected]


# ---------------------------------------------------------------------------
# Multi-value lists: string ("a/b/c"), python list, mixed shortname/numeric,
# and preserved duplicates
# ---------------------------------------------------------------------------
def test_multivalue_string_list():
    assert _param("t/z/u", OD_PL) == ["130", "129", "131"]


def test_multivalue_python_list():
    assert _param(["2t", "msl"], OD_SFC) == ["167", "151"]


def test_multivalue_mixed_shortname_and_numeric():
    assert _param("t/130/z", OD_PL) == ["130", "130", "129"]


def test_multivalue_duplicates_preserved():
    assert _param("t/t/z", OD_PL) == ["130", "130", "129"]


# ---------------------------------------------------------------------------
# Shortname matching is case-insensitive
# ---------------------------------------------------------------------------
@pytest.mark.parametrize("shortname, expected", [("2T", "167"), ("MSL", "151")])
def test_shortname_case_insensitive(shortname, expected):
    assert _param(shortname, OD_SFC) == [expected]


# ---------------------------------------------------------------------------
# 'all' passes through; 'off' unsets (yields no values)
# ---------------------------------------------------------------------------
def test_all_passthrough():
    assert _param("all", OD_PL) == ["all"]


def test_off_yields_no_values():
    assert _param("off", OD_PL) == []


# ---------------------------------------------------------------------------
# Insufficient context: a sparse context that matches no specific rule
# resolves via metkit's default rule
# ---------------------------------------------------------------------------
def test_insufficient_context_uses_default_rule():
    # Only levtype=sfc is supplied; class/stream/type are NOT invented, so the
    # surface-specific rule (which would give 164) does not fire -- 't' falls
    # back to the default 130 rather than to 164.
    assert expand_key("param", "t", context={"levtype": "sfc"}) == ["130"]
    # No context at all: same default-rule fallback.
    assert expand_key("param", "t") == ["130"]


# ---------------------------------------------------------------------------
# Error cases
# ---------------------------------------------------------------------------
def test_unknown_shortname_raises():
    with pytest.raises(MetKitException):
        _param("notaparam", OD_SFC)


def test_out_of_range_numeric_raises():
    with pytest.raises(MetKitException):
        _param("999999", OD_PL)


def test_unmatched_param_table_raises():
    # '174.228' encodes 228*1000+174 = 228174, which is not a known id here.
    with pytest.raises(MetKitException):
        _param("174.228", OD_SFC)


# ---------------------------------------------------------------------------
# Context may be a dict or a MarsRequest, and both agree
# ---------------------------------------------------------------------------
def test_context_dict_and_marsrequest_agree():
    from_dict = _param("t", OD_SFC)

    req = MarsRequest(
        "retrieve", class_="od", stream="oper", type="an", levtype="sfc"
    )
    from_request = expand_key("param", "t", context=req)

    assert from_dict == from_request == ["164"]


# ---------------------------------------------------------------------------
# expand_key matches a full request expansion
# ---------------------------------------------------------------------------
@pytest.mark.parametrize("shortname", ["t", "z", "q", "u", "v"])
def test_parity_with_full_request_expansion(shortname):
    req = MarsRequest(
        "retrieve",
        class_="od",
        stream="oper",
        type="an",
        levtype="pl",
        param=shortname,
        date="-1",
        expver="1",
        step="0",
    )
    assert expand_key("param", shortname, context=OD_PL) == [req.expand()["param"]]
