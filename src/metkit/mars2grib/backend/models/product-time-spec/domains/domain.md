<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Domain Cases

## Overview

Domain classification chooses the absolute support-placement rule applied after
the anchor is already resolved.

Classification is exhaustive and non-prioritized. Exactly one matcher must
return `true`.

## Truth Table

| Case | `isSynoptic` | `regime` | `simulationType` | Result |
|---|---|---|---|---|
| `ForecastDomain` | `false` | any | `Forecast` | support ends at `referenceDateTime + step` and extends backward by the resolved outer range |
| `AnalysisDomain` | `false` | not `AIFS` | `Analysis` | support starts at `referenceDateTime` and extends forward by the resolved outer range |
| `SynopticAnalysisDomain` | `true` | `IFS` | `Analysis` | support starts at exact MARS date/time and ends at the next calendar-month boundary |

## Unsupported States

Examples of unsupported domain-driving combinations are:

- synoptic forecast products;
- synoptic AIFS analysis products;
- non-synoptic AIFS analysis products.
