/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Json.h
/// @brief JSON reading and writing for datamod::Dict, without going through eckit::Value.
///
/// Reading: objects become Dict (key order kept), integers that fit in int64 become Int, other numbers
/// Double, `null` stays Null. Arrays must be homogeneous: integers -> IntList, numbers with at least one
/// non-integer -> DoubleList, strings -> StringList, objects -> DictList, `[]` -> empty IntList.
/// Other arrays (booleans, nulls, nested arrays, mixed kinds) are rejected with eckit::BadValue.
/// Writing: compact JSON in insertion order; doubles with 17 significant digits and always with a
/// fraction or exponent, so they read back as the same Double.

#pragma once

#include <string>
#include <string_view>

#include "metkit/datamod/Dict.h"

namespace metkit::datamod {

/// Parse a JSON document whose root is an object.
Dict parseJsonObject(std::string_view json);

/// Parse any JSON value.
Value parseJson(std::string_view json);

std::string toJson(const Dict& dict);
std::string toJson(const Value& value);

}  // namespace metkit::datamod
