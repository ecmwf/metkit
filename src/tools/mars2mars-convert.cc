/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

///
/// @file mars2mars-convert.cc
/// @brief Tool for converting pre-MTG2 MARS requests to post-MTG2 MARS requests.
///

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/tool/MetkitTool.h"

#include "metkit/mars2mars/api/Mars2Mars.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_mars_request.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"

using namespace eckit;
using namespace eckit::option;

namespace metkit::mars2mars {

//----------------------------------------------------------------------------------------------------------------------

namespace {

using metkit::mars::MarsRequest;

//----------------------------------------------------------------------------------------------------------------------
// Flattening policy
//----------------------------------------------------------------------------------------------------------------------

const std::vector<std::string>& splitKeys() {
    // static const std::vector<std::string> keys = {
    //     "class",
    //     "stream",
    //     "type",
    //     "expver",
    //     "levtype",
    //     "param",
    //     "levelist",
    //     "step",
    //     "date",
    //     "time",
    //     "number",
    //     "chem",
    //     "wavelength",
    //     "timespan",
    //     "hdate",
    //     "htime",
    // };
    static const std::vector<std::string> keys = {
        "levelist",
        "levtype",
        "param",
    };

    return keys;
}

//----------------------------------------------------------------------------------------------------------------------
// Squashing policy
//----------------------------------------------------------------------------------------------------------------------

const std::vector<std::string>& squashKeys() {
    static const std::vector<std::string> keys = {
        "levelist",
        "levtype",
        "param",
    };

    return keys;
}

//----------------------------------------------------------------------------------------------------------------------
// Request signatures
//----------------------------------------------------------------------------------------------------------------------

std::vector<std::string> sortedKeys(const MarsRequest& request) {
    std::vector<std::string> keys;

    for (const auto& parameter : request.parameters()) {
        keys.push_back(parameter.name());
    }

    std::sort(keys.begin(), keys.end());
    return keys;
}

std::string valuesSignature(const std::vector<std::string>& values) {
    std::ostringstream os;

    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            os << "/";
        }

        os << values[i];
    }

    return os.str();
}

std::string squashSignature(const MarsRequest& request, const std::string& squashKey) {
    std::ostringstream signature;

    signature << "verb=" << request.verb() << "|";

    for (const auto& key : sortedKeys(request)) {
        if (key == squashKey) {
            continue;
        }

        const auto& values = request.values(key, true);

        signature << key << "=" << valuesSignature(values) << "|";
    }

    return signature.str();
}

//----------------------------------------------------------------------------------------------------------------------
// Squashing
//----------------------------------------------------------------------------------------------------------------------

std::vector<std::string> uniqueValuesPreserveOrder(const std::vector<MarsRequest>& block,
                                                   const std::string& squashKey) {
    std::vector<std::string> values;
    std::set<std::string> seen;

    for (const auto& request : block) {
        if (!request.has(squashKey)) {
            continue;
        }

        const auto& current = request.values(squashKey);

        for (const auto& value : current) {
            if (seen.insert(value).second) {
                values.push_back(value);
            }
        }
    }

    return values;
}

MarsRequest squashBlock(const std::vector<MarsRequest>& block, const std::string& squashKey) {
    ASSERT(!block.empty());

    if (block.size() == 1) {
        return block.front();
    }

    MarsRequest out = block.front();

    const std::vector<std::string> values = uniqueValuesPreserveOrder(block, squashKey);

    if (!values.empty()) {
        out.values(squashKey, values);
    }

    return out;
}

std::vector<MarsRequest> squashByKey(const std::vector<MarsRequest>& requests, const std::string& squashKey) {
    std::map<std::string, std::vector<MarsRequest>> blocks;

    for (const auto& request : requests) {
        blocks[squashSignature(request, squashKey)].push_back(request);
    }

    std::vector<MarsRequest> out;
    out.reserve(blocks.size());

    for (const auto& block : blocks) {
        out.push_back(squashBlock(block.second, squashKey));
    }

    return out;
}

std::vector<MarsRequest> squashMars2MarsRequests(const std::vector<MarsRequest>& requests) {
    std::vector<MarsRequest> out = requests;

    for (const auto& key : squashKeys()) {
        out = squashByKey(out, key);
    }

    return out;
}

//----------------------------------------------------------------------------------------------------------------------
// Output
//----------------------------------------------------------------------------------------------------------------------

void writeJson(const std::vector<MarsRequest>& requests, std::ostream& out) {
    using metkit::mars2mars::utils::dict_traits::dict_to_json;

    out << "[\n";

    for (std::size_t i = 0; i < requests.size(); ++i) {
        out << "  " << dict_to_json(requests[i]);

        if (i + 1 != requests.size()) {
            out << ",";
        }

        out << "\n";
    }

    out << "]\n";
}

void writeMars(const std::vector<MarsRequest>& requests, std::ostream& out) {
    for (const auto& request : requests) {
        request.dump(out);
        out << "\n";
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace

//----------------------------------------------------------------------------------------------------------------------

class Mars2MarsConvert final : public metkit::MetkitTool {
public:

    Mars2MarsConvert(int argc, char** argv);
    ~Mars2MarsConvert() override = default;

private:

    int minimumPositionalArguments() const override { return 1; }

    void init(const CmdArgs& args) override;
    void execute(const CmdArgs& args) override;
    void usage(const std::string& tool) const override;

    void processPath(const eckit::PathName& path, std::vector<metkit::mars::MarsRequest>& outputRequests) const;
    void processFile(const eckit::PathName& path, std::vector<metkit::mars::MarsRequest>& outputRequests) const;

private:

    std::string outFile_;
    std::string format_ = "json";
};

//----------------------------------------------------------------------------------------------------------------------

Mars2MarsConvert::Mars2MarsConvert(int argc, char** argv) : metkit::MetkitTool(argc, argv) {

    options_.push_back(new SimpleOption<std::string>("out", "Output filename. Defaults to stdout."));

    options_.push_back(new SimpleOption<std::string>("format", "Output format: json or mars. Default: json."));
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::init(const CmdArgs& args) {
    args.get("out", outFile_);
    args.get("format", format_);
    args.get("porcelain", porcelain_);

    if (format_ != "json" && format_ != "mars") {
        throw eckit::UserError("Invalid value for --format: `" + format_ + "`. Expected `json` or `mars`.", Here());
    }
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] request1 [request2 ...]" << std::endl
                << std::endl
                << "Convert pre-MTG2 MARS requests to post-MTG2 MARS requests." << std::endl
                << std::endl
                << "The tool:" << std::endl
                << "  1. reads one or more MARS request files" << std::endl
                << "  2. expands each request" << std::endl
                << "  3. flattens each request into scalar point requests" << std::endl
                << "  4. converts each point through Mars2Mars" << std::endl
                << "  5. conservatively squashes converted points over levelist, levtype, param" << std::endl
                << "  6. writes either JSON or MARS request syntax" << std::endl
                << std::endl
                << "Options:" << std::endl
                << "  --out=<file>        Output filename. Defaults to stdout." << std::endl
                << "  --format=json|mars  Output format. Defaults to json." << std::endl
                << std::endl
                << "Examples:" << std::endl
                << "=========" << std::endl
                << tool << " input.mars" << std::endl
                << tool << " --format=json input.mars" << std::endl
                << tool << " --format=mars input.mars" << std::endl
                << tool << " --format=json --out output.json input1.mars input2.mars" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::processPath(const eckit::PathName& path,
                                   std::vector<metkit::mars::MarsRequest>& outputRequests) const {
    if (path.isDir()) {
        std::vector<eckit::PathName> files;
        std::vector<eckit::PathName> directories;

        path.children(files, directories);

        std::sort(files.begin(), files.end());
        std::sort(directories.begin(), directories.end());

        for (const auto& file : files) {
            processPath(file, outputRequests);
        }

        for (const auto& directory : directories) {
            processPath(directory, outputRequests);
        }

        return;
    }

    processFile(path, outputRequests);
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::processFile(const eckit::PathName& path,
                                   std::vector<metkit::mars::MarsRequest>& outputRequests) const {
    if (!porcelain_) {
        Log::info() << "Processing: " << path << std::endl;
    }

    std::ifstream in(path.asString().c_str());

    if (!in) {
        throw eckit::UserError("Cannot open MARS request file: " + path.asString(), Here());
    }

    const std::vector<metkit::mars::MarsRequest> requests = metkit::mars::MarsRequest::parse(in);

    metkit::mars::MarsExpansion expand(true, true);
    Mars2Mars converter;

    for (const auto& request : requests) {
        const metkit::mars::MarsRequest expanded = expand.expand(request);

        const std::vector<metkit::mars::MarsRequest> points = expanded.split(splitKeys());

        std::vector<metkit::mars::MarsRequest> convertedPoints;
        convertedPoints.reserve(points.size());

        for (const auto& point : points) {
            const auto result = converter.convert(point);
            convertedPoints.push_back(result.mars);
        }

        const std::vector<metkit::mars::MarsRequest> squashed = squashMars2MarsRequests(convertedPoints);

        outputRequests.insert(outputRequests.end(), squashed.begin(), squashed.end());
    }
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::execute(const CmdArgs& args) {
    std::unique_ptr<std::ofstream> outFile;

    if (!outFile_.empty()) {
        outFile.reset(new std::ofstream(outFile_.c_str(), std::ios::out | std::ios::binary));

        if (!*outFile) {
            throw eckit::UserError("Cannot open output file: " + outFile_, Here());
        }
    }

    std::ostream& out = outFile ? *outFile : std::cout;

    std::vector<metkit::mars::MarsRequest> outputRequests;

    for (size_t i = 0; i < args.count(); ++i) {
        processPath(args(i), outputRequests);
    }

    if (format_ == "json") {
        writeJson(outputRequests, out);
    }
    else if (format_ == "mars") {
        writeMars(outputRequests, out);
    }
    else {
        throw eckit::UserError("Invalid output format: `" + format_ + "`", Here());
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2mars

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    metkit::mars2mars::Mars2MarsConvert tool(argc, argv);
    return tool.start();
}