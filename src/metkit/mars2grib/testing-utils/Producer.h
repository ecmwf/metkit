/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#pragma once

#include <memory>
#include <optional>
#include <string>

namespace metkit::mars2grib::testing_utils::run_tests {

class Producer {
public:
    explicit Producer(const std::string& archivePath);
    ~Producer();

    Producer(const Producer&)            = delete;
    Producer& operator=(const Producer&) = delete;
    Producer(Producer&&)                 = delete;
    Producer& operator=(Producer&&)      = delete;

    std::optional<std::string> readNextRecord();

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace metkit::mars2grib::testing_utils::run_tests
