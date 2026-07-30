"""
Pydantic models for pymetkit parameter metadata.
"""

from __future__ import annotations

from typing import Annotated, Any

from pydantic import BaseModel, Field, field_validator, model_validator


class MarsRequestContext(BaseModel):
    """A MARS-key context that (partly) selects a parameter id.

    This mirrors a single matcher rule from ``share/metkit/params.yaml``. Its
    schema is kept **separate** from :class:`ParameterEntry` so the context
    contract can evolve independently and so user-supplied context schemas can
    validate against it. All fields are optional because ``params.yaml`` rules
    do not always constrain every key (e.g. ``levtype`` is occasionally absent).
    """

    model_config = {"populate_by_name": True, "extra": "allow"}

    class_: "str | None" = Field(
        default=None,
        alias="class",
        description="MARS class (e.g. 'od', 'ai')",
    )
    stream: "str | None" = Field(
        default=None, description="MARS stream (e.g. 'oper', 'enfo')"
    )
    type: "str | None" = Field(
        default=None, description="MARS type (e.g. 'fc', 'cf')"
    )
    levtype: "str | None" = Field(
        default=None, description="MARS level type (e.g. 'sfc', 'pl')"
    )


class ParameterEntry(BaseModel):
    """
    A single entry from the ECMWF parameter database.

    Accepts both the canonical field names produced by ``_normalise`` and the
    raw aliases that may appear in YAML or API responses.
    """

    model_config = {"populate_by_name": True, "extra": "allow"}

    id: int = Field(..., description="Numeric ECMWF/GRIB parameter ID")

    shortname: str = Field(
        ...,
        alias="shortname",
        description="Short name (e.g. 't', 'tp')",
    )

    longname: str = Field(
        ...,
        alias="longname",
        description="Human-readable long name (e.g. 'Temperature')",
    )

    units: str = Field(
        default="unknown",
        description="Physical units string (e.g. 'K', 'm s**-1')",
    )

    table: "int | None" = Field(
        default=None,
        description="GRIB parameter table the id encodes to (e.g. 128, 228)",
    )

    mars_request_context: list[MarsRequestContext] = Field(
        default_factory=list,
        description=(
            "MARS-key contexts (from params.yaml) in which this paramid "
            "appears; used to disambiguate shortname collisions"
        ),
    )

    origin_ids: list[int] = Field(
        default_factory=list,
        description="WMO originating centre IDs associated with this parameter",
    )

    access_ids: list[str] = Field(
        default_factory=list,
        description="Access category tags (e.g. 'dissemination', 'research')",
    )

    # ------------------------------------------------------------------
    # Validators
    # ------------------------------------------------------------------

    @field_validator("id", mode="before")
    @classmethod
    def coerce_id_to_int(cls, v: Any) -> int:
        try:
            return int(v)
        except (TypeError, ValueError) as exc:
            raise ValueError(f"'id' must be convertible to int, got {v!r}") from exc

    @field_validator("shortname", mode="before")
    @classmethod
    def normalise_shortname_key(cls, v: Any) -> str:
        if v is None or str(v).strip() == "":
            raise ValueError("'shortname' must be a non-empty string")
        return str(v)

    @field_validator("longname", mode="before")
    @classmethod
    def normalise_longname_key(cls, v: Any) -> str:
        if v is None or str(v).strip() == "":
            raise ValueError("'longname' must be a non-empty string")
        return str(v)

    @field_validator("units", mode="before")
    @classmethod
    def default_empty_units(cls, v: Any) -> str:
        if v is None or str(v).strip() == "":
            return "unknown"
        return str(v)

    @field_validator("table", mode="before")
    @classmethod
    def coerce_table(cls, v: Any) -> "int | None":
        if v is None or str(v).strip() == "":
            return None
        return int(v)

    @field_validator("origin_ids", mode="before")
    @classmethod
    def coerce_origin_ids(cls, v: Any) -> list[int]:
        if v is None:
            return []
        return [int(x) for x in v]

    @field_validator("access_ids", mode="before")
    @classmethod
    def coerce_access_ids(cls, v: Any) -> list[str]:
        if v is None:
            return []
        return [str(x) for x in v]

    @model_validator(mode="before")
    @classmethod
    def _normalise_aliases(cls, data: Any) -> Any:
        """Accept legacy key spellings from raw YAML / API payloads."""
        if not isinstance(data, dict):
            return data
        d = dict(data)
        # shortname aliases
        if "shortname" not in d:
            for alias in ("shortName", "short_name"):
                if alias in d:
                    d["shortname"] = d.pop(alias)
                    break
        # longname aliases
        if "longname" not in d:
            for alias in ("longName", "long_name", "name"):
                if alias in d:
                    d["longname"] = d.pop(alias)
                    break
        return d
