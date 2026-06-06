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

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
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

class Mars2MarsConvert final : public metkit::MetkitTool {
public:
    Mars2MarsConvert(int argc, char** argv);

    ~Mars2MarsConvert() override = default;

private:
    void init(const CmdArgs& args) override;
    void execute(const CmdArgs& args) override;
    void usage(const std::string& tool) const override;

    int minimumPositionalArguments() const override { return 1; }

    void processFile(const eckit::PathName& path, std::ostream& out, bool& firstJsonObject) const;

    static const std::vector<std::string>& splitKeys();

private:
    std::string outFile_;
};

//----------------------------------------------------------------------------------------------------------------------

Mars2MarsConvert::Mars2MarsConvert(int argc, char** argv) :
    metkit::MetkitTool(argc, argv) {

    options_.push_back(new SimpleOption<std::string>("out", "Output JSON filename. Defaults to stdout."));
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::init(const CmdArgs& args) {
    args.get("out", outFile_);
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::usage(const std::string& tool) const {
    Log::info()
        << "Usage: " << tool << " [options] request1 [request2 ...]" << std::endl
        << std::endl
        << "Convert pre-MTG2 MARS requests to post-MTG2 MARS requests." << std::endl
        << std::endl
        << "The tool:" << std::endl
        << "  1. reads one or more MARS request files" << std::endl
        << "  2. expands each request" << std::endl
        << "  3. flattens each request into scalar MARS points" << std::endl
        << "  4. converts each point through Mars2Mars" << std::endl
        << "  5. writes the converted points as a JSON array" << std::endl
        << std::endl
        << "Examples:" << std::endl
        << "=========" << std::endl
        << tool << " input.mars" << std::endl
        << tool << " --out output.json input.mars" << std::endl
        << tool << " --out output.json input1.mars input2.mars" << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

const std::vector<std::string>& Mars2MarsConvert::splitKeys() {
    static const std::vector<std::string> keys = {
        "class",
        "stream",
        "type",
        "expver",
        "levtype",
        "param",
        "levelist",
        "step",
        "date",
        "time",
        "number",
        "chem",
        "wavelength",
        "timespan",
        "hdate",
        "htime",
    };

    return keys;
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::processFile(
    const eckit::PathName& path,
    std::ostream& out,
    bool& firstJsonObject) const {

    using metkit::mars2mars::utils::dict_traits::dict_to_json;

    std::ifstream in(path.asString().c_str());

    if (!in) {
        throw eckit::UserError(
            "Cannot open MARS request file: " + path.asString(),
            Here());
    }

    std::vector<metkit::mars::MarsRequest> requests =
        metkit::mars::MarsRequest::parse(in);

    metkit::mars::MarsExpansion expand(true, true);
    Mars2Mars converter;

    for (const auto& request : requests) {
        const metkit::mars::MarsRequest expanded = expand.expand(request);

        const std::vector<metkit::mars::MarsRequest> points =
            expanded.split(splitKeys());

        for (const auto& point : points) {
            const auto [converted, misc] = converter.convert(point);

            if (!firstJsonObject) {
                out << ",\n";
            }

            out << "  " << dict_to_json(converted);

            firstJsonObject = false;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

void Mars2MarsConvert::execute(const CmdArgs& args) {
    std::unique_ptr<std::ofstream> outFile;

    if (!outFile_.empty()) {
        outFile = std::make_unique<std::ofstream>(
            outFile_.c_str(),
            std::ios::out | std::ios::binary);

        outFile->exceptions(std::ios::badbit);
    }

    std::ostream& out = outFile ? *outFile : std::cout;

    bool firstJsonObject = true;

    out << "[\n";

    for (size_t i = 0; i < args.count(); ++i) {
        processFile(args(i), out, firstJsonObject);
    }

    out << "\n]\n";
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars2mars

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    metkit::mars2mars::Mars2MarsConvert tool(argc, argv);
    return tool.start();
}