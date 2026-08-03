"""Tests for :func:`pymetkit.expand_key` covering the more esoteric corners of
the MARS language: per-keyword default ``by`` steps, descending ranges, range
synonyms, date arithmetic (relative/leap/day-of-year), value normalisation and
context-sensitive keywords.

These complement the basic ``expand_key`` cases in ``test_marsrequest.py``.
"""

from datetime import datetime, timedelta

import pytest

from pymetkit import MarsRequest, MetKitException, expand_key


def _relative_date(offset_days: int) -> str:
    return (datetime.today() + timedelta(days=offset_days)).strftime("%Y%m%d")


yesterday = _relative_date(-1)
today = _relative_date(0)


# ---------------------------------------------------------------------------
# Per-keyword default "by" step
# ---------------------------------------------------------------------------
# The default increment of a "to" range is defined per keyword by the language,
# not globally. "step" defaults to by=12, while plain numeric keywords default
# to by=1. A range whose span is smaller than the default step collapses to just
# the start value.
@pytest.mark.parametrize(
    "keyword, value, expected",
    [
        ("step", "0/to/24", ["0", "12", "24"]),  # step default by == 12
        ("step", "0/to/6", ["0"]),  # span < default step -> only start
        ("step", "0/to/24/by/6", ["0", "6", "12", "18", "24"]),  # explicit by
        ("number", "1/to/5", ["1", "2", "3", "4", "5"]),  # numeric default by == 1
    ],
)
def test_default_by_is_per_keyword(keyword, value, expected):
    assert expand_key(keyword, value) == expected


# ---------------------------------------------------------------------------
# Descending ranges and invalid steps
# ---------------------------------------------------------------------------
# A range where the end is below the start is descending. The direction can be
# implied (positive by) or given explicitly with a negative by.
@pytest.mark.parametrize(
    "value, expected",
    [
        ("10/to/1/by/1", ["10", "9", "8", "7", "6", "5", "4", "3", "2", "1"]),
        ("10/to/1/by/-1", ["10", "9", "8", "7", "6", "5", "4", "3", "2", "1"]),
        ("5/to/5/by/1", ["5"]),  # degenerate range
    ],
)
def test_descending_ranges(value, expected):
    assert expand_key("number", value) == expected


def test_zero_step_raises():
    with pytest.raises(MetKitException):
        expand_key("number", "1/to/10/by/0")


# ---------------------------------------------------------------------------
# Range keyword synonyms: "to"/"t0" and case-insensitivity
# ---------------------------------------------------------------------------
@pytest.mark.parametrize("sep", ["to", "TO", "To", "t0", "T0"])
def test_to_synonyms_are_case_insensitive(sep):
    assert expand_key("number", f"1/{sep}/5") == ["1", "2", "3", "4", "5"]


def test_by_keyword_is_case_insensitive():
    assert expand_key("number", "0/to/9/BY/3") == ["0", "3", "6", "9"]


# ---------------------------------------------------------------------------
# Time normalisation
# ---------------------------------------------------------------------------
# Times normalise to HHMM. Bare hour values are widened, ':' separators are
# stripped, and ranges may step in minutes.
@pytest.mark.parametrize(
    "value, expected",
    [
        ("6", ["0600"]),
        ("06:30", ["0630"]),
        ("0/to/12/by/6", ["0000", "0600", "1200"]),
        ("0000/to/0100/by/0030", ["0000", "0030", "0100"]),
    ],
)
def test_time_normalisation(value, expected):
    assert expand_key("time", value) == expected


@pytest.mark.parametrize("value", ["notatime", "2500"])
def test_invalid_time_raises(value):
    with pytest.raises(MetKitException):
        expand_key("time", value)


# ---------------------------------------------------------------------------
# Date arithmetic
# ---------------------------------------------------------------------------
def test_relative_dates():
    assert expand_key("date", "-1") == [yesterday]
    assert expand_key("date", "0") == [today]


def test_date_year_day_of_year():
    # yyyy-ddd expands to the yyyymmdd of the ddd-th day of the year.
    assert expand_key("date", "2018-23") == ["20180123"]


def test_date_range_alternate_format():
    # Check that expansion respects alternate date formats
    assert expand_key("date", "2020-01-01/to/2020-01-03") == [
        "2020-01-01",
        "2020-01-02",
        "2020-01-03",
    ]


def test_date_range_crosses_leap_day():
    # 2020 is a leap year, so 29 Feb is included.
    assert expand_key("date", "20200228/to/20200302") == [
        "20200228",
        "20200229",
        "20200301",
        "20200302",
    ]


def test_date_range_with_step():
    assert expand_key("date", "20200101/to/20200110/by/2") == [
        "20200101",
        "20200103",
        "20200105",
        "20200107",
        "20200109",
    ]


@pytest.mark.parametrize("value", ["jan", "feb", "dec"])
def test_month_name_climatology_passthrough(value):
    # Bare month names are climatology values and are kept as-is (lower-cased).
    assert expand_key("date", value) == [value]


# ---------------------------------------------------------------------------
# Numeric / expver value normalisation
# ---------------------------------------------------------------------------
# Float keywords use metkit's canonical form: leading/trailing zeros stripped.
@pytest.mark.parametrize(
    "value, expected",
    [
        ("0.5/to/2.0/by/0.5", [".5", "1", "1.5", "2"]),
        ("2.0", ["2"]),
        ("-0.5", ["-0.5"]),
        ("1000", ["1000"]),
    ],
)
def test_float_normalisation(value, expected):
    assert expand_key("levelist", value) == expected


# expver is zero-padded to four characters but left untouched when alphanumeric.
@pytest.mark.parametrize(
    "value, expected",
    [("1", "0001"), ("0001", "0001"), ("abcd", "abcd")],
)
def test_expver_normalisation(value, expected):
    assert expand_key("expver", value) == [expected]


# ---------------------------------------------------------------------------
# Duplicates, "all", and lists without ranges
# ---------------------------------------------------------------------------
def test_duplicates_preserved():
    # 'step' allows duplicate values; they are not de-duplicated.
    assert expand_key("step", "1/1/2") == ["1", "1", "2"]


def test_all_passthrough():
    assert expand_key("step", "all") == ["all"]


def test_plain_list_without_to():
    assert expand_key("step", "0/6/12/18") == ["0", "6", "12", "18"]


def test_empty_value_yields_empty_list():
    assert expand_key("step", "") == []


# ---------------------------------------------------------------------------
# Keyword resolution: aliases, prefixes, unknown / ambiguous keys
# ---------------------------------------------------------------------------
@pytest.mark.parametrize("alias", ["levelist", "level", "levellist", "leve"])
def test_keyword_aliases_resolve(alias):
    assert expand_key(alias, "1/to/3") == ["1", "2", "3"]


def test_param_resolves_with_context():
    # 'param' is resolved transparently via a full multi-pass expansion.
    assert expand_key("param", "130", context={"levtype": "pl"}) == ["130"]
    assert expand_key("param", "t", context={"levtype": "pl"}) == ["130"]
    # context changes the result: 't' is 130 on pressure levels, 164 on surface
    sfc = {"class": "od", "stream": "oper", "type": "an", "levtype": "sfc"}
    assert expand_key("param", "t", context=sfc) == ["164"]


def test_unknown_keyword_raises():
    with pytest.raises(MetKitException):
        expand_key("notakeyword", "1")


def test_ambiguous_prefix_raises():
    # 'l' matches several keywords and cannot be resolved unambiguously.
    with pytest.raises(MetKitException):
        expand_key("l", "1")


# ---------------------------------------------------------------------------
# Verb selection
# ---------------------------------------------------------------------------
def test_verb_selects_language():
    # step's default by==12 holds under the 'archive' verb too.
    assert expand_key("step", "0/to/24", verb="archive") == ["0", "12", "24"]


# ---------------------------------------------------------------------------
# Context-sensitive keywords
# ---------------------------------------------------------------------------
def test_context_as_dict():
    assert expand_key("levelist", "1000/to/850/by/50", context={"levtype": "pl"}) == [
        "1000",
        "950",
        "900",
        "850",
    ]


def test_context_as_marsrequest():
    context = MarsRequest("retrieve", levtype="ml")
    assert expand_key("levelist", "1/to/3", context=context) == ["1", "2", "3"]


# ---------------------------------------------------------------------------
# Input shapes: strings, lists, ints
# ---------------------------------------------------------------------------
@pytest.mark.parametrize(
    "value, expected",
    [
        ("1/to/5", ["1", "2", "3", "4", "5"]),
        (["1", "to", "5"], ["1", "2", "3", "4", "5"]),
        ([1, "to", 5], ["1", "2", "3", "4", "5"]),
        ([0, 6, 12], ["0", "6", "12"]),
        (5, ["5"]),
    ],
)
def test_input_shapes(value, expected):
    assert expand_key("number", value) == expected


# ---------------------------------------------------------------------------
# Strict validation
# ---------------------------------------------------------------------------
def test_strict_rejects_invalid_value():
    with pytest.raises(MetKitException):
        expand_key("time", "notatime", strict=True)
