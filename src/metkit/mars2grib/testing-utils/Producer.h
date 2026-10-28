/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <optional>
#include <string>

#include "eckit/blockfile/RecordReader.h"

namespace metkit::mars2grib::testing_utils::run_tests {

/// True when `path` ends in `.zst` or `.zstd`.
bool isZstdPath(const std::string& path);

/// Reads test-case records one at a time from a block file, a zstd-compressed JSONL file or a
/// plain JSONL file (detected from the content). Empty lines are skipped.
class Producer {
public:

    explicit Producer(const std::string& path);

    std::optional<std::string> readNextRecord();

private:

    eckit::blockfile::RecordReader reader_;
};

}  // namespace metkit::mars2grib::testing_utils::run_tests
