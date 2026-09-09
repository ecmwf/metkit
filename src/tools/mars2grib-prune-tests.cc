/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <cstddef>
#include <exception>
#include <fstream>
#include <initializer_list>
#include <string>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/TestsFilter.h"
#include "metkit/tool/MetkitTool.h"

namespace {

eckit::LocalConfiguration requiredObject(const eckit::LocalConfiguration& root, const std::string& key,
                                         std::size_t lineNumber) {
    if (!root.has(key) || !root.isSubConfiguration(key)) {
        throw eckit::UserError("Test-case record at line " + std::to_string(lineNumber) + " requires object `" + key +
                                   "`",
                               Here());
    }
    return root.getSubConfiguration(key);
}

class Mars2GribPruneTestsTool final : public metkit::MetkitTool {
public:
    Mars2GribPruneTestsTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("input-file", "Input JSONL test-case file"));
        options_.push_back(new eckit::option::SimpleOption<std::string>("output-file", "Output JSONL test-case file"));
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "filter-perturbed-forecast", "Keep only perturbation number 1 for perturbed forecasts (default true)"));
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "filter-model-level", "Keep only model level 1 when levelist is present (default true)"));
        options_.push_back(new eckit::option::SimpleOption<bool>(
            "filter-frequency-direction", "Keep only frequency=1 and direction=1 when both are present (default true)"));
    }

private:
    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    std::string inputPath_;
    std::string outputPath_;
    eckit::LocalConfiguration filterOptions_;
};

void Mars2GribPruneTestsTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("input-file") || !args.has("output-file")) {
        throw eckit::UserError("--input-file and --output-file are required", Here());
    }

    args.get("input-file", inputPath_);
    args.get("output-file", outputPath_);
    if (inputPath_.empty() || outputPath_.empty()) {
        throw eckit::UserError("--input-file and --output-file must not be empty", Here());
    }
    if (inputPath_ == outputPath_) {
        throw eckit::UserError("--input-file and --output-file must be different", Here());
    }

    for (const std::string name : {"filter-perturbed-forecast", "filter-model-level", "filter-frequency-direction"}) {
        bool enabled = true;
        if (args.has(name)) {
            args.get(name, enabled);
        }
        filterOptions_.set(name, enabled);
    }
}

void Mars2GribPruneTestsTool::execute(const eckit::option::CmdArgs&) {
    std::ifstream input{inputPath_};
    if (!input) {
        throw eckit::UserError("Unable to open input test-case file `" + inputPath_ + "`", Here());
    }

    std::ofstream output{outputPath_, std::ios::out | std::ios::trunc};
    if (!output) {
        throw eckit::UserError("Unable to open output test-case file `" + outputPath_ + "`", Here());
    }

    metkit::mars2grib::testing_utils::TestsFilter filter{filterOptions_};
    std::string record;
    std::size_t lineNumber = 0;
    while (std::getline(input, record)) {
        ++lineNumber;
        if (!record.empty() && record.back() == '\r') {
            record.pop_back();
        }
        if (record.empty()) {
            throw eckit::UserError("Empty test-case record at line " + std::to_string(lineNumber), Here());
        }

        try {
            const eckit::LocalConfiguration root{eckit::YAMLConfiguration{record}};
            const auto mars = requiredObject(root, "mars", lineNumber);
            const auto misc = requiredObject(root, "misc", lineNumber);
            const auto opt  = requiredObject(root, "opt", lineNumber);
            (void)requiredObject(root, "out", lineNumber);

            if (filter.filter(mars, misc, opt)) {
                output << record << '\n';
                if (!output) {
                    throw eckit::Exception("Unable to write output test-case file `" + outputPath_ + "`", Here());
                }
            }
        }
        catch (const std::exception& exception) {
            throw eckit::UserError("Unable to process test-case record at line " + std::to_string(lineNumber) + ": " +
                                       exception.what(),
                                   Here());
        }
        catch (...) {
            throw eckit::UserError("Unable to process test-case record at line " + std::to_string(lineNumber) +
                                       ": unknown exception",
                                   Here());
        }
    }

    if (input.bad()) {
        throw eckit::Exception("Error while reading input test-case file `" + inputPath_ + "`", Here());
    }

    output.close();
    if (!output) {
        throw eckit::Exception("Unable to complete output test-case file `" + outputPath_ + "`", Here());
    }
}

void Mars2GribPruneTestsTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage: " << tool
                       << " --input-file <input.jsonl> --output-file <output.jsonl> "
                          "[--filter-perturbed-forecast=<bool>] [--filter-model-level=<bool>] "
                          "[--filter-frequency-direction=<bool>]"
                       << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribPruneTestsTool tool(argc, argv);
    return tool.start();
}
