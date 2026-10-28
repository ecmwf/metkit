/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <memory>
#include <string>
#include <string_view>

namespace metkit::mars2grib::testing_utils::run_tests {

/// Writes JSONL records to a plain or zstd-compressed file.
///
/// The format is chosen from the file extension (see `isZstdPath`). Records are buffered;
/// call `close()` to flush and detect write errors (the destructor closes silently).
class JsonlWriter {
public:

    explicit JsonlWriter(const std::string& path, int zstdLevel = 9);
    ~JsonlWriter();

    JsonlWriter(const JsonlWriter&)            = delete;
    JsonlWriter& operator=(const JsonlWriter&) = delete;
    JsonlWriter(JsonlWriter&&)                 = delete;
    JsonlWriter& operator=(JsonlWriter&&)      = delete;

    /// Append `record` followed by a newline.
    void write(std::string_view record);

    void close();

private:

    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace metkit::mars2grib::testing_utils::run_tests
