/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/Producer.h"

#include "eckit/filesystem/PathName.h"

namespace metkit::mars2grib::testing_utils::run_tests {

namespace {

bool endsWith(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() && value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

}  // namespace

bool isZstdPath(const std::string& path) {
    return endsWith(path, ".zst") || endsWith(path, ".zstd");
}

Producer::Producer(const std::string& path) : reader_{eckit::PathName{path}} {}

std::optional<std::string> Producer::readNextRecord() {
    std::string record;
    if (!reader_.next(record)) {
        return std::nullopt;
    }
    return record;
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
