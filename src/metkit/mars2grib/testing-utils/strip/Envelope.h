/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Envelope.h
/// @brief A test-case record travelling through the stripping pipeline.
///
/// The reading rank wraps each JSONL record with what is expensive to compute and needed by the owner
/// rank: the routing hash, the time representation (anchor/domain/shape type of the product time
/// spec) and whether the record is the first emission of its time representation (step equal to the
/// outer time range, as mars2grib-prune-tests-otr). The record text itself is never modified.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "metkit/datamod/Dict.h"

namespace metkit::mars2grib::testing_utils::strip {

inline constexpr const char* missingValue = "missing";

struct Envelope {
    std::string record;
    std::uint64_t route{0};
    std::string anchorType{missingValue};
    std::string domainType{missingValue};
    std::string shapeType{missingValue};
    bool firstEmission{true};
    bool classified{false};

    /// "anchor/domain/shape".
    std::string timeRepresentation() const;
};

/// Build the envelope of a record (reading rank): parses only mars, misc and opt, classifies the time
/// spec and computes the routing hash of (class, stream, type, levtype, anchor, domain, shape).
/// Records whose time spec cannot be computed get `missing` and `classified == false`.
Envelope makeEnvelope(std::string record);

/// Append `envelope` to `buffer` (byte format used for the MPI exchange).
void appendEnvelope(std::vector<char>& buffer, const Envelope& envelope);

/// Read the envelope at `position` and advance it; false at the end of `buffer`.
bool readEnvelope(const std::vector<char>& buffer, std::size_t& position, Envelope& envelope);

/// The owner rank's view of a test: mars, misc and opt parsed with datamod (`out` is not parsed).
struct TestView {
    const Envelope& envelope;
    metkit::datamod::Dict mars;
    metkit::datamod::Dict misc;
    metkit::datamod::Dict opt;

    static TestView parse(const Envelope& envelope);
};

}  // namespace metkit::mars2grib::testing_utils::strip
