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
/// @file grib1-to-grib2.cc
/// @brief CLI tool for converting grib1 to grib2 files.
///

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/FileHandle.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/log/Log.h"
#include "eckit/message/Message.h"
#include "eckit/message/Reader.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/EckitTool.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/grib2mars/api/Grib2Mars.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2mars/api/Mars2Mars.h"

using namespace eckit;
using namespace eckit::option;

//----------------------------------------------------------------------------------------------------------------------

class Grib1ToGrib2Tool final : public eckit::EckitTool {
public:

    Grib1ToGrib2Tool(int argc, char** argv);
    ~Grib1ToGrib2Tool() override = default;

private:

    int minimumPositionalArguments() const override { return 2; }
    void init(const CmdArgs& args) override;
    void execute(const CmdArgs& args) override;
    void usage(const std::string& tool) const override;
};

//----------------------------------------------------------------------------------------------------------------------

Grib1ToGrib2Tool::Grib1ToGrib2Tool(int argc, char** argv) : eckit::EckitTool(argc, argv) {}

void Grib1ToGrib2Tool::init(const CmdArgs& args) {}

void Grib1ToGrib2Tool::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] input output" << std::endl
                << std::endl
                << "Convert GRIB1 to GRIB2" << std::endl
                << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

std::unique_ptr<metkit::codes::CodesHandle> readCodesHandle(eckit::message::Message& msg) {
    std::unique_ptr<eckit::DataHandle> dataHandle{msg.readHandle()};

    eckit::MemoryHandle* memoryHandle = reinterpret_cast<eckit::MemoryHandle*>(dataHandle.get());

    if (memoryHandle == nullptr) {
        throw eckit::UserError("Could not read eckit::Message", Here());
    }

    const auto* data = reinterpret_cast<const uint8_t*>(memoryHandle->data());
    const auto size  = static_cast<std::size_t>(memoryHandle->size());

    return metkit::codes::codesHandleFromMessageCopy(metkit::codes::Span<const uint8_t>(data, size));
}

eckit::LocalConfiguration mergeLocalConfigs(const eckit::LocalConfiguration& base,
                                            const eckit::LocalConfiguration& overwrite) {
    eckit::LocalConfiguration result{base};
    for (const auto& key : overwrite.keys()) {
        if (overwrite.isString(key)) {
            result.set(key, overwrite.getString(key));
        }
        else if (overwrite.isIntegral(key)) {
            result.set(key, overwrite.getLong(key));
        }
        else if (overwrite.isFloatingPoint(key)) {
            result.set(key, overwrite.getDouble(key));
        }
        else if (overwrite.isBoolean(key)) {
            result.set(key, overwrite.getBool(key));
        }
        else if (overwrite.isFloatingPointList(key)) {
            result.set(key, overwrite.getDoubleVector(key));
        }
        else {
            throw eckit::NotImplemented("Unexpected type for '" + key + "'", Here());
        }
    }
    return result;
}

//----------------------------------------------------------------------------------------------------------------------

void Grib1ToGrib2Tool::execute(const CmdArgs& args) {

    // Handles to conversion libraries
    metkit::grib2mars::Grib2Mars grib2mars;
    metkit::mars2mars::Mars2Mars mars2mars;
    metkit::mars2grib::Mars2Grib mars2grib;

    auto inputPath  = eckit::PathName(args(0));
    auto outputPath = eckit::PathName(args(1));

    if (!inputPath.exists()) {
        throw eckit::UserError("Input file path '" + inputPath.path() + "' does not exist!", Here());
    }

    // Remove the output file if it already exists, otherwise we are not able to write to it.
    if (outputPath.exists()) {
        remove(outputPath.asString().c_str());
    }
    auto outputFile = eckit::FileHandle(outputPath, true);
    outputFile.openForWrite(0);


    eckit::message::Reader msgReader{args(0)};
    eckit::message::Message msg;
    while ((msg = msgReader.next())) {
        auto codesHandle = readCodesHandle(msg);

        // Read the MARS/Misc dictionary from the input GRIB sample
        const auto originalMarsMisc = grib2mars.convert<eckit::LocalConfiguration>(*codesHandle);

        // Read the values from the input GRIB sample
        const std::vector<double> values = codesHandle->getDoubleArray("values");

        // Apply mappings to convert pre-MTG2 MARS/Misc to post-MTG2 MARS/Misc
        const auto mappedMarsMisc = mars2mars.convert<eckit::LocalConfiguration>(originalMarsMisc.mars);

        auto mars = mappedMarsMisc.mars;
        auto misc = mergeLocalConfigs(mappedMarsMisc.misc, originalMarsMisc.misc);

        // Encode into GRIB2 using mars2grib encoder
        const auto newSample = mars2grib.encode(values, mars, misc);

        // Write newly encoded GRIB2 sample to the output file
        outputFile.write(newSample->messageData().data(), newSample->messageData().size());
    }

    outputFile.close();
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    Grib1ToGrib2Tool tool(argc, argv);
    return tool.start();
}
