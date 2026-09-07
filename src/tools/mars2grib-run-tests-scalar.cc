/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <optional>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/Consumer.h"
#include "metkit/tool/MetkitTool.h"

namespace {

class Mars2GribRunTestsScalarTool final : public metkit::MetkitTool {
public:
    Mars2GribRunTestsScalarTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("test-cases", "Plain JSONL test-case file"));
    }

private:
    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    std::string testCasesPath_;
};

void Mars2GribRunTestsScalarTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("test-cases")) {
        throw eckit::UserError("--test-cases is required", Here());
    }
    args.get("test-cases", testCasesPath_);
}

void Mars2GribRunTestsScalarTool::execute(const eckit::option::CmdArgs&) {
    std::ifstream input{testCasesPath_};
    if (!input) {
        throw eckit::UserError("Unable to open test-case file `" + testCasesPath_ + "`", Here());
    }

    using metkit::mars2grib::testing_utils::run_tests::Consumer;
    using metkit::mars2grib::testing_utils::run_tests::ConsumerOptions;
    Consumer consumer{ConsumerOptions{true, std::nullopt}};

    std::size_t passed = 0;
    std::size_t failed = 0;
    std::string record;
    while (std::getline(input, record)) {
        if (!record.empty() && record.back() == '\r') {
            record.pop_back();
        }
        if (consumer.run(record)) {
            ++passed;
        }
        else {
            ++failed;
        }
    }
    if (input.bad()) {
        throw eckit::Exception("Error while reading test-case file `" + testCasesPath_ + "`", Here());
    }

    const std::size_t total       = passed + failed;
    const double passedPercentage = total == 0 ? 0.0 : 100.0 * static_cast<double>(passed) / static_cast<double>(total);
    const double failedPercentage = total == 0 ? 0.0 : 100.0 * static_cast<double>(failed) / static_cast<double>(total);
    eckit::Log::info() << "TOT TESTS: " << total << '\n'
                       << "PASSED: " << passed << ", " << std::fixed << std::setprecision(2) << passedPercentage
                       << "%\n"
                       << "FAILED: " << failed << ", " << failedPercentage << '%' << std::endl;

    if (failed != 0) {
        throw eckit::Exception(std::to_string(failed) + " mars2grib tests failed", Here());
    }
}

void Mars2GribRunTestsScalarTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage: " << tool << " --test-cases <test-cases.jsonl>" << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribRunTestsScalarTool tool(argc, argv);
    return tool.start();
}
