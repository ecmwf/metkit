/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <initializer_list>
#include <set>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/mpi/Comm.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars2grib/testing-utils/TestsFilter.h"
#include "metkit/tool/MetkitTool.h"

namespace {

constexpr std::size_t rootRank  = 0;
constexpr int validationTag     = 100;
constexpr int validationTextTag = 101;
constexpr int workCountTag      = 200;
constexpr int workLengthTag     = 201;
constexpr int workTextTag       = 202;

void sendString(const eckit::mpi::Comm& comm, const std::string& value, int destination, int lengthTag, int textTag) {
    const std::size_t length = value.size();
    comm.send(length, destination, lengthTag);
    if (length != 0) {
        comm.send(value.data(), length, destination, textTag);
    }
}

std::string receiveString(const eckit::mpi::Comm& comm, int source, int lengthTag, int textTag) {
    std::size_t length = 0;
    comm.receive(length, source, lengthTag);
    std::string value(length, '\0');
    if (length != 0) {
        comm.receive(value.data(), length, source, textTag);
    }
    return value;
}

std::vector<eckit::PathName> readInputPaths(const eckit::PathName& listPath) {
    std::ifstream input{listPath.asString()};
    if (!input) {
        throw eckit::Exception("Unable to open input file list `" + listPath.asString() + "`", Here());
    }

    std::vector<eckit::PathName> paths;
    std::set<eckit::PathName> uniquePaths;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (line.empty()) {
            throw eckit::Exception(
                "Empty filename at line " + std::to_string(lineNumber) + " in `" + listPath.asString() + "`", Here());
        }
        if (line.back() == '\r') {
            throw eckit::Exception(
                "Invalid carriage return at line " + std::to_string(lineNumber) + " in `" + listPath.asString() + "`",
                Here());
        }
        if (line.find('\0') != std::string::npos) {
            throw eckit::Exception(
                "Invalid filename at line " + std::to_string(lineNumber) + " in `" + listPath.asString() + "`", Here());
        }

        const eckit::PathName path{line};
        if (path.fullName().asString() != line) {
            throw eckit::Exception("Filename at line " + std::to_string(lineNumber) + " in `" + listPath.asString() +
                                       "` is not a full path: `" + line + "`",
                                   Here());
        }
        if (!uniquePaths.insert(path).second) {
            throw eckit::Exception("Duplicate filename at line " + std::to_string(lineNumber) + " in `" +
                                       listPath.asString() + "`: `" + line + "`",
                                   Here());
        }
        paths.emplace_back(path);
    }
    if (input.bad()) {
        throw eckit::Exception("Error while reading input file list `" + listPath.asString() + "`", Here());
    }
    if (paths.empty()) {
        throw eckit::Exception("Input file list `" + listPath.asString() + "` is empty", Here());
    }
    return paths;
}

std::vector<std::vector<eckit::PathName>> distributePaths(const std::vector<eckit::PathName>& paths,
                                                          std::size_t taskCount) {
    std::vector<std::vector<eckit::PathName>> assignments(taskCount);
    const std::size_t pathsPerTask = paths.size() / taskCount;
    const std::size_t remainder    = paths.size() % taskCount;
    std::size_t offset             = 0;
    for (std::size_t rank = 0; rank < taskCount; ++rank) {
        const std::size_t count = pathsPerTask + (rank < remainder ? 1 : 0);
        assignments[rank].insert(assignments[rank].end(), paths.begin() + offset, paths.begin() + offset + count);
        offset += count;
    }
    return assignments;
}

class Mars2GribPruneTestsMpiTool final : public metkit::MetkitTool {
public:

    Mars2GribPruneTestsMpiTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<std::string>("input-file-list",
                                                                        "Text file containing one full path per line"));
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

    eckit::PathName inputFileList_;
    eckit::LocalConfiguration filterOptions_;
};

void Mars2GribPruneTestsMpiTool::init(const eckit::option::CmdArgs& args) {
    if (!args.has("input-file-list")) {
        throw eckit::Exception("--input-file-list is required", Here());
    }

    std::string inputFileList;
    args.get("input-file-list", inputFileList);
    if (inputFileList.empty()) {
        throw eckit::Exception("--input-file-list must not be empty", Here());
    }
    inputFileList_ = eckit::PathName{inputFileList};

    for (const std::string name : {"filter-perturbed-forecast", "filter-model-level", "filter-frequency-direction"}) {
        bool enabled = true;
        if (args.has(name)) {
            args.get(name, enabled);
        }
        filterOptions_.set(name, enabled);
    }
}

void Mars2GribPruneTestsMpiTool::execute(const eckit::option::CmdArgs&) {
    eckit::mpi::Comm& comm      = eckit::mpi::comm();
    const std::size_t rank      = comm.rank();
    const std::size_t taskCount = comm.size();

    std::vector<std::vector<eckit::PathName>> assignments;
    std::string validationError;
    if (rank == rootRank) {
        try {
            assignments = distributePaths(readInputPaths(inputFileList_), taskCount);
        }
        catch (const eckit::Exception& exception) {
            validationError = exception.what();
        }

        const int valid = validationError.empty() ? 1 : 0;
        for (std::size_t destination = 1; destination < taskCount; ++destination) {
            comm.send(valid, static_cast<int>(destination), validationTag);
            if (!valid) {
                sendString(comm, validationError, static_cast<int>(destination), validationTextTag,
                           validationTextTag + 1);
            }
        }
    }
    else {
        int valid = 0;
        comm.receive(valid, static_cast<int>(rootRank), validationTag);
        if (!valid) {
            validationError = receiveString(comm, static_cast<int>(rootRank), validationTextTag, validationTextTag + 1);
        }
    }
    if (!validationError.empty()) {
        throw eckit::Exception(validationError, Here());
    }

    comm.barrier();

    std::vector<eckit::PathName> work;
    if (rank == rootRank) {
        work = assignments[rootRank];
        for (std::size_t destination = 1; destination < taskCount; ++destination) {
            const std::size_t count = assignments[destination].size();
            comm.send(count, static_cast<int>(destination), workCountTag);
            for (const auto& path : assignments[destination]) {
                sendString(comm, path.asString(), static_cast<int>(destination), workLengthTag, workTextTag);
            }
        }
    }
    else {
        std::size_t count = 0;
        comm.receive(count, static_cast<int>(rootRank), workCountTag);
        work.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            work.emplace_back(receiveString(comm, static_cast<int>(rootRank), workLengthTag, workTextTag));
        }
    }

    comm.barrier();

    std::string processingError;
    try {
        for (const auto& inputPath : work) {
            eckit::PathName outputPath{inputPath};
            outputPath += ".pruned";
            metkit::mars2grib::testing_utils::pruneTestsFile(inputPath, outputPath, filterOptions_);
        }
    }
    catch (const eckit::Exception& exception) {
        processingError = exception.what();
    }
    const int localFailure = processingError.empty() ? 0 : 1;
    std::vector<int> failures(taskCount);
    comm.allGather(localFailure, failures.begin(), failures.end());
    comm.barrier();

    if (std::any_of(failures.begin(), failures.end(), [](int failed) { return failed != 0; })) {
        if (!processingError.empty()) {
            throw eckit::Exception("Rank " + std::to_string(rank) + " failed while pruning: " + processingError,
                                   Here());
        }
        throw eckit::Exception("One or more MPI ranks failed while pruning", Here());
    }
}

void Mars2GribPruneTestsMpiTool::usage(const std::string& tool) const {
    eckit::Log::info() << "Usage: " << tool
                       << " --input-file-list <files.txt> [--filter-perturbed-forecast=<bool>] "
                          "[--filter-model-level=<bool>] [--filter-frequency-direction=<bool>]"
                       << std::endl;
}

}  // namespace

int main(int argc, char** argv) {
    Mars2GribPruneTestsMpiTool tool(argc, argv);
    const int status = tool.start();
    eckit::mpi::finaliseAllComms();
    return status;
}
