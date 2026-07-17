"""Tests for :func:`pymetkit.expand_key` that specifically demonstrate
*context dependence*: keywords whose expansion depends on the values of other
keys in the request.

``expand_key`` performs single-pass expansion, so context matters here in two
ways:

* keywords backed by a context-switched (``mixed``) type pick a different value
  set / parser depending on the supplied ``context`` (e.g. ``model``, ``domain``,
  ``bcmodel``, ``tilescheme``);
* a value can be valid in one context and rejected in another, and every rule of
  a context (all of its keys) must match for that context to apply.

The final section documents the deliberate limitation that second-pass,
rule-based resolution (e.g. ``param``) is *not* applied by ``expand_key`` -- that
still requires a full (scoped) request expansion.
"""

import pytest

from pymetkit import MarsRequest, MetKitException, expand_key


# Contexts reused across the tests.
MS_IT = {"class": "ms", "country": "it"}  # selects the Italy model
DT_EXTREMES = {"class": "d1", "dataset": "on-demand-extremes-dt"}


# ---------------------------------------------------------------------------
# A value can be valid only within a matching context
# ---------------------------------------------------------------------------
# 'model' is a context-switched keyword. "wam" is only a member of the
# class=ms/country=it value set; with no context it falls back to the generic
# model enum, which does not contain it, so expansion fails.
@pytest.mark.parametrize(
    "keyword, value, context",
    [
        ("model", "wam", MS_IT),
        ("domain", "italy", MS_IT),
        ("domain", "euroatl", MS_IT),
        ("bcmodel", "gme", MS_IT),
        ("icmodel", "ifs", MS_IT),
    ],
)
def test_value_valid_only_with_matching_context(keyword, value, context):
    # Valid when the context matches ...
    assert expand_key(keyword, value, context=context) == [value]
    # ... and rejected when the context is absent.
    with pytest.raises(MetKitException):
        expand_key(keyword, value)


# ---------------------------------------------------------------------------
# The same value expands differently per context
# ---------------------------------------------------------------------------
# Without the class=ms/country=it context, "mediterranean"/"europe" resolve
# against the generic domain enum to their single-letter codes; within that
# context they resolve to the named Italy domains.
@pytest.mark.parametrize(
    "value, generic, in_context",
    [
        ("mediterranean", "m", "mediterranean"),
        ("europe", "e", "europe"),
    ],
)
def test_same_value_expands_differently_per_context(value, generic, in_context):
    assert expand_key("domain", value) == [generic]
    assert expand_key("domain", value, context=MS_IT) == [in_context]


# ---------------------------------------------------------------------------
# Every rule of a context must match (partial context is not enough)
# ---------------------------------------------------------------------------
# The class=ms/country=it context requires BOTH keys; supplying only one does not
# activate its value set.
@pytest.mark.parametrize(
    "partial_context",
    [
        {"class": "ms"},  # missing country
        {"country": "it"},  # missing class
        {"class": "od", "country": "it"},  # wrong class
        None,  # no context at all
    ],
)
def test_partial_context_does_not_activate_rule(partial_context):
    with pytest.raises(MetKitException):
        expand_key("model", "wam", context=partial_context)


def test_full_context_activates_rule():
    assert expand_key("model", "wam", context=MS_IT) == ["wam"]


# ---------------------------------------------------------------------------
# Context-independent values for a context-sensitive keyword (contrast)
# ---------------------------------------------------------------------------
# Even though 'model'/'domain' are context-sensitive, some values live in the
# generic fallback set and therefore expand the same way regardless of context.
@pytest.mark.parametrize(
    "keyword, value, expected",
    [
        ("model", "glob", "global"),  # 'glob' is an alias for 'global'
        ("model", "ecmf", "ecmf"),
        ("domain", "g", "g"),
    ],
)
@pytest.mark.parametrize("context", [None, MS_IT, {"class": "ti"}])
def test_fallback_values_are_context_independent(keyword, value, expected, context):
    assert expand_key(keyword, value, context=context) == [expected]


# ---------------------------------------------------------------------------
# Aliases / normalisation available only within a context
# ---------------------------------------------------------------------------
# 'tilescheme' only resolves inside the class=d1 / dataset=on-demand-extremes-dt
# context. There the numeric form is an alias for the named value.
def test_alias_resolves_only_in_context():
    assert expand_key("tilescheme", "simple", context=DT_EXTREMES) == ["simple"]
    # numeric alias maps to the named value ...
    assert expand_key("tilescheme", "1", context=DT_EXTREMES) == ["simple"]
    assert expand_key("tilescheme", "2", context=DT_EXTREMES) == ["granular"]
    # ... but the same values are meaningless without the context.
    with pytest.raises(MetKitException):
        expand_key("tilescheme", "simple")


# ---------------------------------------------------------------------------
# Context can itself depend on a context-sensitive keyword
# ---------------------------------------------------------------------------
# 'tilescheme' needs class=d1 AND dataset=on-demand-extremes-dt. 'dataset' is
# itself a context-switched keyword, so this exercises a two-level context.
@pytest.mark.parametrize(
    "context",
    [
        {"class": "d1"},  # missing dataset
        {"dataset": "on-demand-extremes-dt"},  # missing class
    ],
)
def test_tilescheme_requires_both_context_keys(context):
    with pytest.raises(MetKitException):
        expand_key("tilescheme", "granular", context=context)


# ---------------------------------------------------------------------------
# Context accepted as both a dict and a MarsRequest
# ---------------------------------------------------------------------------
def test_context_dict_and_marsrequest_agree():
    from_dict = expand_key("domain", "euroatl", context=MS_IT)

    req = MarsRequest("retrieve", class_="ms", country="it")
    from_request = expand_key("domain", "euroatl", context=req)

    assert from_dict == from_request == ["euroatl"]


# ---------------------------------------------------------------------------
# Documented limitation: second-pass (pass2) resolution is NOT applied
# ---------------------------------------------------------------------------
# 'param' is resolved in a second pass by matching many other keys. expand_key
# performs single-pass expansion only, so a symbolic param name is passed through
# unchanged even when the relevant context keys are supplied ...
def test_param_pass2_not_applied_by_expand_key():
    param_context = {"levtype": "pl", "stream": "oper", "type": "an", "class": "od"}
    assert expand_key("param", "t") == ["t"]
    assert expand_key("param", "t", context=param_context) == ["t"]


# ... whereas a full (scoped) request expansion does resolve it.
def test_param_resolved_by_full_request_expansion():
    req = MarsRequest(
        "retrieve",
        class_="od",
        stream="oper",
        type="an",
        levtype="pl",
        param="t",
        date="-1",
        expver="1",
        step="0",
    )
    assert req.expand()["param"] == "130"
