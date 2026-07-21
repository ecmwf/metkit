<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Anchor Cases

## Overview

Anchor classification chooses the direct-source regime used to construct the
three ordered anchor datetimes:

- `labelDateTime`
- `initialConditionsDateTime`
- `referenceDateTime`

Classification is exhaustive and non-prioritized. Exactly one matcher must
return `true`.

## Truth Table

| Case | MARS `date` | MARS `time` | MARS `hdate` | MARS `year` | MARS `month` | Result |
|---|---|---|---|---|---|---|
| `ForecastAnalysis` | present | optional | absent | absent | absent | all three anchors equal `DateTime(date, time or 00:00:00)` |
| `Hindcast` | present | optional | present | absent | absent | `label = DateTime(hdate, 00:00:00)`, `initial = reference = DateTime(date, time or 00:00:00)` |
| `SeasonalClimate` | absent | absent | absent | present | present | all three anchors equal first day of `(year, month)` at `00:00:00` |

## Invalid Source States

Examples of invalid or unsupported source combinations are:

- `time` present without `date`;
- exactly one of `year` or `month` present;
- `date` or `hdate` present together with `year` / `month`;
- complete absence of all anchor source families.
