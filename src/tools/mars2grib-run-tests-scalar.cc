/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "eckit/blockfile/Filter.h"
#include "eckit/blockfile/Format.h"
#include "eckit/blockfile/Reader.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/Consumer.h"
#include "metkit/mars2grib/testing-utils/Producer.h"
#include "metkit/mars2grib/testing-utils/RunTestsOptions.h"
#include "metkit/tool/MetkitTool.h"

namespace {

using metkit::mars2grib::testing_utils::run_tests::CommonRunTestsOptions;

class Mars2GribRunTestsScalarTool final : public metkit::MetkitTool {
public:

    Mars2GribRunTestsScalarTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "test-cases", "Test-case block file, or JSONL file (zstd-compressed or plain)"));
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "select", "Block files only: run blocks matching e.g. class=od,stream=enfo/oper,type=pf"));
        options_.push_back(new eckit::option::SimpleOption<std::string>(
            "blocks", "Block files only: run these blocks, e.g. 0,3,5-7 (combined with --select if both are given)"));
        metkit::mars2grib::testing_utils::run_tests::addCommonRunTestsOptions(options_);
    }

private:

    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    std::string testCasesPath_;
    std::optional<std::string> select_;
    std::optional<std::string> blocks_;
    CommonRunTestsOptions common_;
};

void Mars2GribRunTestsScalarTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("test-cases")) {
        throw eckit::UserError("--test-cases is required", Here());
    }
    args.get("test-cases", testCasesPath_);
    if (args.has("select")) {
        if (!eckit::blockfile::isBlockFile(eckit::PathName{testCasesPath_})) {
            throw eckit::UserError("--select needs a block-file input", Here());
        }
        std::string select;
        args.get("select", select);
        select_ = std::move(select);
    }
    if (args.has("blocks")) {
        if (!eckit::blockfile::isBlockFile(eckit::PathName{testCasesPath_})) {
            throw eckit::UserError("--blocks needs a block-file input", Here());
        }
        std::string blocks;
        args.get("blocks", blocks);
        blocks_ = std::move(blocks);
    }
    common_ = metkit::mars2grib::testing_utils::run_tests::readCommonRunTestsOptions(args);
}

void Mars2GribRunTestsScalarTool::execute(const eckit::option::CmdArgs&) {
    // Stack files only with reports: each one is copied into its report and deleted.
    if (common_.failureReports) {
        metkit::mars2grib::testing_utils::run_tests::enableErrorStackFiles();
    }
    using namespace metkit::mars2grib::testing_utils::run_tests;
    using Clock = std::chrono::steady_clock;

    const auto start = Clock::now();

    ConsumerOptions options;
    options.logErrorDetails = true;
    options.parser          = common_.parser;
    options.mode            = common_.mode;
    options.decodeMarsOnly  = common_.decodeMarsOnly;
    options.failureReports  = common_.failureReports;
    Consumer consumer{std::move(options)};

    std::size_t passed = 0;
    std::size_t failed = 0;
    const auto runOne  = [&](const std::string& record) {
        if (consumer.run(record)) {
            ++passed;
        }
        else {
            ++failed;
        }
    };

    if (eckit::blockfile::isBlockFile(eckit::PathName{testCasesPath_})) {
        const eckit::blockfile::Reader reader{eckit::PathName{testCasesPath_}};
        const auto filter = select_ ? eckit::blockfile::Filter::parse(*select_) : eckit::blockfile::Filter{};
        std::vector<std::size_t> ids;
        if (blocks_) {
            for (std::size_t id : parseBlockIds(*blocks_, reader.numberOfBlocks())) {
                if (filter.matches(reader.keys(), reader.blocks()[id])) {
                    ids.push_back(id);
                }
            }
        }
        else {
            ids = reader.select(filter);
        }
        std::string record;
        for (std::size_t id : ids) {
            reader.readBlock(id).forEachRecord([&](std::string_view view) {
                record.assign(view.data(), view.size());
                runOne(record);
            });
        }
        eckit::Log::info() << "BLOCKS: " << ids.size() << " of " << reader.numberOfBlocks() << std::endl;
    }
    else {
        Producer producer{testCasesPath_};
        while (auto record = producer.readNextRecord()) {
            runOne(*record);
        }
    }
    const auto wall = Clock::now() - start;

    const std::size_t total       = passed + failed;
    const double passedPercentage = total == 0 ? 0.0 : 100.0 * static_cast<double>(passed) / static_cast<double>(total);
    const double failedPercentage = total == 0 ? 0.0 : 100.0 * static_cast<double>(failed) / static_cast<double>(total);
    eckit::Log::info() << "TOT TESTS: " << total << '\n'
                       << "PASSED: " << passed << ", " << std::fixed << std::setprecision(2) << passedPercentage
                       << "%\n"
                       << "FAILED: " << failed << ", " << failedPercentage << '%' << std::defaultfloat << std::endl;

    if (common_.failureReports && failed != 0) {
        eckit::Log::info() << "Failure reports written under " << common_.failureReports->directory << std::endl;
    }

    if (common_.timings) {
        // Time not spent in a test is reading/decompressing the input.
        const ConsumerTimings& timings = consumer.timings();
        const auto inTests = timings.parse + timings.encode + timings.compare + timings.decode + timings.failureOutput;
        printConsumerTimings(eckit::Log::info(), timings, total, 1, wall - inTests, wall);
    }

    if (failed != 0) {
        throw eckit::Exception(std::to_string(failed) + " mars2grib tests failed", Here());
    }
}

void Mars2GribRunTestsScalarTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage: " << tool
                       << " --test-cases <file> [--select class=..,stream=..,type=..] [--blocks <ids>] (block input) "
                          "[--failure-reports [--failure-reports-dir <dir>] "
                          "[--failure-reports-schema <schema>]] [--mode encode|decode|roundtrip] [--decode-mars-only] "
                          "[--timings] [--yaml-parser]"
                       << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribRunTestsScalarTool tool(argc, argv);
    return tool.start();
}
