/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/option/SimpleOption.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/value/Value.h"
#include "metkit/tool/MetkitTool.h"
#include "metkit/gribjump/GribInfo.h"
#include "metkit/gribjump/GribHandleData.h"
#include <chrono>
#include <fstream>
#include "eckit/utils/StringTools.h"
// using namespace metkit;
using namespace metkit::gribjump;

//----------------------------------------------------------------------------------------------------------------------

class GribJump : public metkit::MetkitTool {

public:

    GribJump(int argc, char **argv) : metkit::MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("extract", "Extract info from grib header to write to binary metadata file (set by -o)"));
        options_.push_back(new eckit::option::SimpleOption<std::string>("meta", "Name of binary metadata file to write/read to/from (default: <input_grib_name>.bin)"));
        options_.push_back(new eckit::option::SimpleOption<bool>("query", "Query data range from grib file"));
        options_.push_back(new eckit::option::SimpleOption<std::string>("msgs", "Which message(s) (from 0 to N-1) of the N messages in grib file to query (comma separated string)"));
        options_.push_back(new eckit::option::SimpleOption<bool>("time", "Append timing information to gribjump-timing.txt"));
    }

private: // methods

    virtual int minimumPositionalArguments() const { return 1; }
    virtual void execute(const eckit::option::CmdArgs& args);
    virtual void init(const eckit::option::CmdArgs& args);
    virtual void usage(const std::string& tool) const;

private: // members
    bool doExtract_ = false;
    bool doQuery_ = false;
    bool doRange_ = false;
    bool doTime_ = false;
    eckit::PathName gribFileName_;
    eckit::PathName binFileName_;
    std::vector<size_t> msgids_;
    size_t singleIndex_;
    std::vector<std::tuple<size_t, size_t>> rangesVector_;
};

void GribJump::usage(const std::string &tool) const {
    eckit::Log::info() << std::endl
                        << "Usage: " << tool << " [options] [input_grib_file] [min0] [max0] [min1] ... "
                        << std::endl;

    eckit::Log::info() << "Examples:" << std::endl
                        << "=========" << std::endl
                        << std::endl
                        << "e.g. Process and extract metadata from data.grib to data.grib.bin:" << std::endl
                        << tool << " --extract data.grib" << std::endl
                        << std::endl
                        << "e.g. Retrieve data in range [12, 45) and [56, 789) from the 0th, 1st and 10th, message in data.grib." << std::endl
                        << tool << " --query --msgs=0,1,10 data.grib 12 45 56 789" << std::endl
                        << std::endl;
}

void GribJump::init(const eckit::option::CmdArgs& args) {
    doExtract_ = args.getBool("extract", false);
    doQuery_ = args.getBool("query", false);
    doTime_ = args.getBool("time", false);
    std::string msgs = args.getString("msgs", "0");
    std::vector<std::string> s = eckit::StringTools::split(",", msgs);
    for (auto m : s){
        msgids_.push_back(std::stoi(m));
    }

    gribFileName_ = args(0);
    binFileName_ = args.getString("meta", gribFileName_.baseName() + ".bin");
    ASSERT(gribFileName_.exists());

    doExtract_ |= !binFileName_.exists(); // if bin doesn't exist, extract before query

    ASSERT(!(doTime_ && doExtract_ && doQuery_)); // Do not time both extract and query at the same time.

    if (!doQuery_) return;

    if (args.count() == 2){
        doRange_ = false;
        std::cout << "Query single point" << std::endl;
        std::cout << "index: " << args(1) << std::endl;
        singleIndex_ = std::stoi(args(1));
    } else {
        doRange_ = true;
        // note ranges must have a start and end
        ASSERT(args.count() % 2 == 1);
        std::cout << "Query range(s): ";
        for (int i = 1; i < args.count(); i+=2){
            std::cout << args(i) << "-" << args(i+1) << ", ";
            rangesVector_.push_back({std::make_tuple(std::stoi(args(i)), std::stoi(args(i+1)))});
        }
        std::cout << std::endl;
    }
}

void GribJump::execute(const eckit::option::CmdArgs& args) {
    auto startTime = std::chrono::high_resolution_clock::now();
    auto endTime = std::chrono::high_resolution_clock::now();
    GribInfo gribInfo;
    GribHandleData dataSource(gribFileName_);

    if (doExtract_) {
        std::cout << "Build jump info from " << gribFileName_ << std::endl;
        startTime = std::chrono::high_resolution_clock::now();
        gribInfo = dataSource.extractMetadata(binFileName_);
        endTime = std::chrono::high_resolution_clock::now();

        std::cout << gribInfo << std::endl;
    }

    if (doQuery_){
        for (auto msg : msgids_){
            std::cout << "Grib file: " << gribFileName_ << ", jump info file: " << binFileName_ << ", msg id: " << msg << std::endl;
            gribInfo.fromBinary(binFileName_, msg);
        
            ASSERT(gribInfo.ready());

            if (doRange_){
                startTime = std::chrono::high_resolution_clock::now();
                std::vector<double> v = gribInfo.extractAtIndexRangeOfRanges(dataSource, rangesVector_);
                endTime = std::chrono::high_resolution_clock::now();
                std::cout << "Value: " << v << std::endl;
            }
            else{
                size_t index = std::stoi(args(1));
                std::cout << "Query index " << index << " in " << gribFileName_ << std::endl;
                double v = gribInfo.extractAtIndex(dataSource, index);
                std::cout << "Value: " << v << std::endl;
            }

        }

    }

    if (doTime_){
        // filename, msgid, ranges, time
        std::chrono::duration<double> elapsed = endTime - startTime;
        std::cout << "Elapsed time: " << elapsed.count() << " s" << std::endl;
        std::ofstream timingFile;
        timingFile.open("gribjump-timing.txt", std::ios_base::app);
        // also priint the rangesvector in the next column
        timingFile << "\"" << gribFileName_ << "\",";
        timingFile << "\"" << msgids_ << "\",";
        timingFile << "\"";
        for (auto r : rangesVector_){
            timingFile << std::get<0>(r) << "-" << std::get<1>(r);
            if (r != rangesVector_.back()){
                timingFile << ",";
            }
        }
        timingFile << "\"," << elapsed.count() << std::endl;


        timingFile.close();
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    GribJump tool(argc,argv);
    return tool.start();
}
