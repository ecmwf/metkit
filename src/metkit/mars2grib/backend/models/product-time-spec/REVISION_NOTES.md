<!--
(C) Copyright 2026- ECMWF and individual contributors.

This software is licensed under the terms of the Apache Licence Version 2.0
which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
In applying this licence, ECMWF does not waive the privileges and immunities
granted to it by virtue of its status as an intergovernmental organisation nor
does it submit to any jurisdiction.
-->

# Revision notes — V4

- Adopted the complete ECMWF Apache 2.0 licence notice used by metkit headers.
- Expanded file and function documentation following the deduction-header style.
- Removed `detail/ExceptionUtils.h` completely.
- Added a local `using` declaration for `Mars2GribModelException` in every function.
- Added direct `std::throw_with_nested(...)` rethrowing at every function boundary.
- Attached `input.to_json()` whenever `ProductTimeSpecInput` is available.
- Used the `(reason, Here())` exception constructor in low-level helpers.
- Removed stage-based exception adapter logic.
- Converted all aggregate construction to C++17-compatible syntax.
- Added an explicit C++17 compile-time requirement.
- Preserved leaf builders and semantically named matcher conditions.
- Verified the complete header set using `-std=c++17 -Wall -Wextra -pedantic-errors` against repository-type stubs.
