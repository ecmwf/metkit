/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <initializer_list>
#include <string>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/TestsFilter.h"
#include "metkit/tool/MetkitTool.h"

namespace {

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
            "filter-frequency-direction",
            "Keep only frequency=1 and direction=1 when both are present (default true)"));
    }

private:

    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    eckit::PathName inputPath_;
    eckit::PathName outputPath_;
    eckit::LocalConfiguration filterOptions_;
};

void Mars2GribPruneTestsTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("input-file") || !args.has("output-file")) {
        throw eckit::Exception("--input-file and --output-file are required", Here());
    }

    std::string inputPath;
    std::string outputPath;
    args.get("input-file", inputPath);
    args.get("output-file", outputPath);
    if (inputPath.empty() || outputPath.empty()) {
        throw eckit::Exception("--input-file and --output-file must not be empty", Here());
    }
    inputPath_  = eckit::PathName{inputPath};
    outputPath_ = eckit::PathName{outputPath};
    if (inputPath_ == outputPath_) {
        throw eckit::Exception("--input-file and --output-file must be different", Here());
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
    metkit::mars2grib::testing_utils::pruneTestsFile(inputPath_, outputPath_, filterOptions_);
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
