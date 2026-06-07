/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * Licensed under the Apache Licence Version 2.0.
 */

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/Log.h"
#include "eckit/message/Message.h"
#include "eckit/message/Reader.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/log/JSON.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/grib2mars/api/Grib2Mars.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/tool/MetkitTool.h"

using namespace eckit;
using namespace eckit::option;

namespace metkit::grib2mars {

namespace {

struct Result {
    metkit::mars::MarsRequest mars;
    eckit::LocalConfiguration misc;
};

std::unique_ptr<metkit::codes::CodesHandle>
readCodesHandle(eckit::message::Message& msg) {
    std::unique_ptr<eckit::DataHandle> dh(msg.readHandle());

    eckit::MemoryHandle* mh =
        dynamic_cast<eckit::MemoryHandle*>(dh.get());

    if (mh == nullptr) {
        throw eckit::UserError(
            "Expected Message::readHandle() to return MemoryHandle",
            Here());
    }

    const uint8_t* data =
        reinterpret_cast<const uint8_t*>(mh->data());

    const std::size_t size =
        static_cast<std::size_t>(mh->size());

    return metkit::codes::codesHandleFromMessageCopy(
        metkit::codes::Span<const uint8_t>(data, size));
}

std::vector<metkit::mars::MarsRequest> expandMarsRequest(
    const metkit::mars::MarsRequest& request) {

    metkit::mars::MarsExpansion expansion(true, true);

    std::vector<metkit::mars::MarsRequest> out;
    out.push_back(expansion.expand(request));

    return out;
}

// Temporary placeholder.
// Better move this into metkit/mars2mars or metkit/grib2mars utility code.
std::vector<Result> squashResults(const std::vector<Result>& input) {
    // TODO:
    //   squash levelist
    //   squash levtype
    //   squash param
    //
    // Use the conservative map-signature algorithm already discussed.
    // Keep misc in the grouping signature.
    return input;
}

void writeMars(const std::vector<Result>& results, std::ostream& out) {
    for (const Result& result : results) {
        result.mars.dump(out);
        out << "\n";
    }
}

void writeJson(const std::vector<Result>& results, std::ostream& out) {
    eckit::JSON json(out);

    json.startList();

    for (const Result& result : results) {
        json.startObject();

        json << "mars";
        result.mars.json(json);

        json << "misc";
        json << result.misc;

        json.endObject();
    }

    json.endList();
}

}  // namespace

class Grib2MarsConvert final : public metkit::MetkitTool {
public:
    Grib2MarsConvert(int argc, char** argv) :
        metkit::MetkitTool(argc, argv) {

        options_.push_back(new SimpleOption<std::string>(
            "out",
            "Output filename. Defaults to stdout."));

        options_.push_back(new SimpleOption<std::string>(
            "format",
            "Output format: json or mars. Default: json."));

        options_.push_back(new SimpleOption<bool>(
            "expand",
            "Expand extracted MARS requests before squashing."));
    }

private:
    int minimumPositionalArguments() const override { return 1; }

    void init(const CmdArgs& args) override {
        args.get("out", out_);
        args.get("format", format_);
        args.get("expand", expand_);

        if (format_ != "json" && format_ != "mars") {
            throw eckit::UserError(
                "Invalid --format value `" + format_ + "`. Expected `json` or `mars`.",
                Here());
        }
    }

    void usage(const std::string& tool) const override {
        Log::info()
            << "Usage: " << tool << " [options] input.grib [input2.grib ...]\n"
            << "\n"
            << "Options:\n"
            << "  --format=json|mars  Output format. Default: json.\n"
            << "  --out=<file>        Output filename. Defaults to stdout.\n"
            << "  --expand            Expand extracted MARS requests before squashing.\n";
    }

    void execute(const CmdArgs& args) override {
        std::vector<Result> results;

        for (std::size_t i = 0; i < args.count(); ++i) {
            processFile(args(i), results);
        }

        const std::vector<Result> squashed = squashResults(results);

        std::unique_ptr<std::ofstream> file;

        if (!out_.empty()) {
            file.reset(new std::ofstream(out_.c_str()));
            if (!*file) {
                throw eckit::UserError("Cannot open output file `" + out_ + "`", Here());
            }
        }

        std::ostream& out = file ? *file : std::cout;

        if (format_ == "mars") {
            writeMars(squashed, out);
            return;
        }

        writeJson(squashed, out);
    }

    void processFile(const eckit::PathName& path, std::vector<Result>& results) const {
        metkit::grib2mars::Grib2Mars converter;

        eckit::message::Reader reader(path);
        eckit::message::Message msg;

        while ((msg = reader.next())) {
            std::unique_ptr<metkit::codes::CodesHandle> grib = readCodesHandle(msg);

            const Grib2MarsResult<metkit::mars::MarsRequest> result =
                converter.convert<metkit::mars::MarsRequest>(*grib);

            if (!expand_) {
                results.push_back(Result{result.mars, result.misc});
                continue;
            }

            const std::vector<metkit::mars::MarsRequest> expanded =
                expandMarsRequest(result.mars);

            for (const metkit::mars::MarsRequest& mars : expanded) {
                results.push_back(Result{mars, result.misc});
            }
        }
    }

private:
    std::string out_;
    std::string format_ = "json";
    bool expand_ = false;
};

}  // namespace metkit::grib2mars

int main(int argc, char** argv) {
    metkit::grib2mars::Grib2MarsConvert tool(argc, argv);
    return tool.start();
}