/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <cstddef>
#include <exception>
#include <fstream>
#include <string>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/api/Mars2GribClassify.h"
#include "metkit/mars2grib/backend/deductions/step.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/tool/MetkitTool.h"

namespace {

eckit::LocalConfiguration requiredObject(const eckit::LocalConfiguration& root, const std::string& key) {
    if (!root.has(key) || !root.isSubConfiguration(key)) {
        throw eckit::Exception("Test-case record requires object `" + key + "`", Here());
    }
    return root.getSubConfiguration(key);
}

class Mars2GribPruneTestsOtrTool final : public metkit::MetkitTool {
public:
    Mars2GribPruneTestsOtrTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("input-file", "Input JSONL test-case file"));
        options_.push_back(new eckit::option::SimpleOption<std::string>("output-file", "Output JSONL test-case file"));
    }

private:
    int numberOfPositionalArguments() const override { return 0; }
    void init(const eckit::option::CmdArgs& args) override;
    void execute(const eckit::option::CmdArgs&) override;
    void usage(const std::string& tool) const override;

    eckit::PathName inputPath_;
    eckit::PathName outputPath_;
};

void Mars2GribPruneTestsOtrTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("input-file") || !args.has("output-file")) {
        throw eckit::UserError("--input-file and --output-file are required", Here());
    }

    std::string inputPath;
    std::string outputPath;
    args.get("input-file", inputPath);
    args.get("output-file", outputPath);
    if (inputPath.empty() || outputPath.empty()) {
        throw eckit::UserError("--input-file and --output-file must not be empty", Here());
    }

    inputPath_  = eckit::PathName{inputPath};
    outputPath_ = eckit::PathName{outputPath};
    if (inputPath_ == outputPath_) {
        throw eckit::UserError("--input-file and --output-file must be different", Here());
    }
    if (outputPath_.exists()) {
        throw eckit::UserError("Output test-case file already exists: `" + outputPath_.asString() + "`", Here());
    }
}

void Mars2GribPruneTestsOtrTool::execute(const eckit::option::CmdArgs&) {
    std::ifstream input{inputPath_.asString()};
    if (!input) {
        throw eckit::UserError("Unable to open input test-case file `" + inputPath_.asString() + "`", Here());
    }

    std::ofstream output{outputPath_.asString(), std::ios::out | std::ios::trunc};
    if (!output) {
        throw eckit::UserError("Unable to open output test-case file `" + outputPath_.asString() + "`", Here());
    }

    std::string record;
    std::size_t lineNumber = 0;
    while (std::getline(input, record)) {
        ++lineNumber;
        if (!record.empty() && record.back() == '\r') {
            record.pop_back();
        }
        if (record.empty()) {
            throw eckit::Exception("Empty test-case record at line " + std::to_string(lineNumber), Here());
        }

        try {
            const eckit::LocalConfiguration root{eckit::YAMLConfiguration{record}};
            const auto mars = requiredObject(root, "mars");
            const auto misc = requiredObject(root, "misc");
            const auto opt  = requiredObject(root, "opt");
            (void)requiredObject(root, "out");

            const auto step = metkit::mars2grib::backend::deductions::resolve_Step_opt(mars, misc, opt);
            bool keep       = !step.has_value() || step->length == 0;
            if (!keep) {
                metkit::mars2grib::Mars2GribClassify classifier{opt};
                keep = step->length == classifier.computeOuterTimeRangeInHours(mars, misc);
            }

            if (keep) {
                output << record << '\n';
                if (!output) {
                    throw eckit::Exception("Unable to write output test-case file `" + outputPath_.asString() + "`",
                                           Here());
                }
            }
        }
        catch (const eckit::Exception& exception) {
            throw eckit::Exception("Unable to process test-case record at line " + std::to_string(lineNumber) +
                                       ": " + exception.what(),
                                   Here());
        }
        catch (const std::exception& exception) {
            throw eckit::Exception("Unable to process test-case record at line " + std::to_string(lineNumber) +
                                       ": " + exception.what(),
                                   Here());
        }
        catch (...) {
            throw eckit::Exception("Unable to process test-case record at line " + std::to_string(lineNumber) +
                                       ": unknown exception",
                                   Here());
        }
    }

    if (input.bad()) {
        throw eckit::Exception("Error while reading input test-case file `" + inputPath_.asString() + "`", Here());
    }

    output.close();
    if (!output) {
        throw eckit::Exception("Unable to complete output test-case file `" + outputPath_.asString() + "`", Here());
    }
}

void Mars2GribPruneTestsOtrTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage: " << tool << " --input-file <input.jsonl> --output-file <output.jsonl>"
                       << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribPruneTestsOtrTool tool(argc, argv);
    return tool.start();
}
