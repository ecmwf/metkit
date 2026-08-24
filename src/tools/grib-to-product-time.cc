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
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
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
#include "eckit/option/SimpleOption.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/grib2mars/api/Grib2Mars.h"
#include "metkit/mars2grib/api/Mars2Grib.h"
#include "metkit/mars2mars/api/Mars2Mars.h"

#include "metkit/mars2grib/backend/models/product-time-spec/ProductTimeSpec.h"

#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_options.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictionary_access_traits.h"

#include "metkit/mars2grib/backend/compile-time-registry-engine/common.h"
#include "metkit/mars2grib/backend/concepts/statistics/statisticsEnum.h"
#include "metkit/mars2grib/backend/concepts/statistics/statisticsMatcher.h"
#include "metkit/mars2grib/backend/tables/typeOfStatisticalProcessing.h"

using namespace eckit;
using namespace eckit::option;

//----------------------------------------------------------------------------------------------------------------------

class GribToProductTime final : public eckit::EckitTool {
public:

    GribToProductTime(int argc, char** argv);
    ~GribToProductTime() override = default;

private:

    int minimumPositionalArguments() const override { return 2; }
    void init(const CmdArgs& args) override;
    void execute(const CmdArgs& args) override;
    void usage(const std::string& tool) const override;

    bool skipDiscipline192_                          = false;
    std::optional<std::string> expver_               = std::nullopt;
    std::optional<long> generatingProcessIdentifier_ = std::nullopt;
};

//----------------------------------------------------------------------------------------------------------------------

GribToProductTime::GribToProductTime(int argc, char** argv) : eckit::EckitTool(argc, argv) {
    options_.push_back(new eckit::option::SimpleOption<bool>("help", "Print this help message"));

    // Input handling
    options_.push_back(
        new eckit::option::SimpleOption<bool>("skip-discipline-192", "Skip discipline 192 input messages"));

    // Override values
    options_.push_back(new eckit::option::SimpleOption<std::string>("expver", "Override expver"));
    options_.push_back(
        new eckit::option::SimpleOption<long>("generatingProcessIdentifier", "Override generatingProcessIdentifier"));
}

void GribToProductTime::init(const CmdArgs& args) {
    skipDiscipline192_ = args.has("skip-discipline-192");

    if (args.has("expver")) {
        std::string expver;
        args.get("expver", expver);
        expver_ = expver;
    }

    if (args.has("generatingProcessIdentifier")) {
        long generatingProcessIdentifier;
        args.get("generatingProcessIdentifier", generatingProcessIdentifier);
        generatingProcessIdentifier_ = generatingProcessIdentifier;
    }
}

void GribToProductTime::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] input output" << std::endl
                << std::endl
                << "Convert (pre-MTG2) GRIB1 to (post-MTG2) GRIB2" << std::endl
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

metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing mars2TypeOfStatisticalProcessing(
    const eckit::LocalConfiguration& mars, const metkit::mars2grib::Options& opt) {

    using metkit::mars2grib::backend::concepts_::statisticsMatcher;
    using metkit::mars2grib::backend::concepts_::StatisticsType;

    const std::size_t typeOfStatisticalProcessingId = statisticsMatcher(mars, opt);

    if (typeOfStatisticalProcessingId == metkit::mars2grib::backend::compile_time_registry_engine::MISSING) {
        // Hack for instant fields
        return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Missing;
    }
    else {
        const StatisticsType sel = static_cast<StatisticsType>(typeOfStatisticalProcessingId);
        switch (sel) {
            case StatisticsType::Average:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Average;
            case StatisticsType::Accumulation:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Accumulation;
            case StatisticsType::Maximum:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Maximum;
            case StatisticsType::Minimum:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Minimum;
            case StatisticsType::DifferenceFromStart:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::DifferenceEndMinusStart;
            case StatisticsType::RootMeanSquare:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::RootMeanSquare;
            case StatisticsType::StandardDeviation:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::StandardDeviation;
            case StatisticsType::Covariance:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Covariance;
            case StatisticsType::DifferenceFromEnd:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::DifferenceStartMinusEnd;
            case StatisticsType::Ratio:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Ratio;
            case StatisticsType::StandardizedAnomaly:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::StandardizedAnomaly;
            case StatisticsType::Summation:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Summation;
            case StatisticsType::ReturnPeriod:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::ReturnPeriod;
            case StatisticsType::Median:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Median;
            case StatisticsType::Severity:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Severity;
            case StatisticsType::Mode:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Mode;
            case StatisticsType::IndexProcessing:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::IndexProcessing;
            case StatisticsType::Default:
                return metkit::mars2grib::backend::tables::TypeOfStatisticalProcessing::Missing;
        }

        throw eckit::UserError("Unexpected StatisticsType value: " + std::to_string(static_cast<std::size_t>(sel)),
                               Here());
    }
}


//----------------------------------------------------------------------------------------------------------------------

void GribToProductTime::execute(const CmdArgs& args) {

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

    metkit::mars2grib::Options opts;

    eckit::message::Reader msgReader{args(0)};
    eckit::message::Message msg;

    std::ofstream outFile(outputPath.asString());

    if (!outFile.is_open()) {
        throw eckit::UserError("Failed to open output file for writing", Here());
    }

    std::string separator = "[";
    while ((msg = msgReader.next())) {
        auto codesHandle = readCodesHandle(msg);

        if (skipDiscipline192_ && codesHandle->getLong("discipline") == 192) {
            continue;
        }

        // Read the MARS/Misc dictionary from the input GRIB sample
        const auto originalMarsMisc = grib2mars.convert<eckit::LocalConfiguration>(*codesHandle);

        // Read the values from the input GRIB sample
        const std::vector<double> values = codesHandle->getDoubleArray("values");

        // Apply mappings to convert pre-MTG2 MARS/Misc to post-MTG2 MARS/Misc
        const auto mappedMarsMisc = mars2mars.convert<eckit::LocalConfiguration>(originalMarsMisc.mars);

        auto mars = mappedMarsMisc.mars;
        auto misc = mergeLocalConfigs(mappedMarsMisc.misc, originalMarsMisc.misc);

        // Override values if specified by the user in the arguments
        if (expver_) {
            mars.set("expver", *expver_);
        }
        if (generatingProcessIdentifier_) {
            misc.set("generatingProcessIdentifier", *generatingProcessIdentifier_);
        }

        // Get type of statistical processing from the MARS dictionary
        const auto innerTypeOfStatisticalProcessing = mars2TypeOfStatisticalProcessing(mars, opts);

        // Generate productTimeSpec
        auto timeSpec =
            metkit::mars2grib::backend::models::ProductTimeSpec(innerTypeOfStatisticalProcessing, mars, misc, opts);

        // Generate a json out dictionary
        {
            std::string jsonMars = metkit::mars2grib::utils::dict_traits::dict_to_json<eckit::LocalConfiguration>(mars);
            std::string jsonMisc = metkit::mars2grib::utils::dict_traits::dict_to_json<eckit::LocalConfiguration>(misc);
            std::string jsonTimeSpec = timeSpec.to_json();

            std::string jsonOut =
                "{ \"mars\": " + jsonMars + ", \"misc\": " + jsonMisc + ", \"productTimeSpec\": " + jsonTimeSpec + " }";

            outFile << separator << jsonOut;
            separator = ",\n";
        }
    }
    outFile << "]" << std::endl;

    outFile.close();
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    GribToProductTime tool(argc, argv);
    return tool.start();
}
