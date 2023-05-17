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
#include "eckit/parser/JSONParser.h"
#include "eckit/value/Value.h"
#include "metkit/tool/MetkitTool.h"
#include "metkit/gribjump/GribInfo.h"
#include "metkit/gribjump/GribHandleData.h"
// using namespace metkit;
using namespace metkit::gribjump;

//----------------------------------------------------------------------------------------------------------------------

class GribJump : public metkit::MetkitTool {

public:

    GribJump(int argc, char **argv) : metkit::MetkitTool(argc, argv) {
        options_.push_back(new eckit::option::SimpleOption<bool>("extract", "Extract info from grib header to write to json file"));
        options_.push_back(new eckit::option::SimpleOption<bool>("query", "Query data range from grib file"));
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
    eckit::PathName gribFileName_;
    eckit::PathName jsonFileName_;
    size_t singleIndex_;
    std::vector<std::tuple<size_t, size_t>> rangesVector;
};

void GribJump::usage(const std::string &tool) const {
    eckit::Log::info() << std::endl
                        << "Usage: " << tool << " [options] [input] [min0] [max0] [min1] ... "
                        << std::endl;

    eckit::Log::info() << "Examples:" << std::endl
                        << "=========" << std::endl
                        << std::endl
                        << "e.g. Process and relevant extract info from data.grib to data.json:" << std::endl
                        << tool << " --extract data.grib" << std::endl
                        << std::endl
                        << "e.g. Retrieve data in range [12, 45) and [56, 789) from data.grib." << std::endl
                        << tool << " --query data.grib 12 45 56 789" << std::endl
                        << std::endl;
}

void GribJump::init(const eckit::option::CmdArgs& args) {
    // XXX: better arg parsing
    doExtract_ = args.getBool("extract", true);
    doQuery_ = args.getBool("query", false);

    gribFileName_ = args(0);
    ASSERT(gribFileName_.exists());

    jsonFileName_ = gribFileName_.baseName() + ".json";
    doExtract_ |= !jsonFileName_.exists(); // if json doesn't exist, extract before query

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
            rangesVector.push_back({std::make_tuple(std::stoi(args(i)), std::stoi(args(i+1)))});
        }
        std::cout << std::endl;
    }
}

void GribJump::execute(const eckit::option::CmdArgs& args) {
    GribInfo gribInfo;
    GribHandleData dataSource(gribFileName_);

    if (doExtract_) {
        std::cout << "Extract from " << gribFileName_ << std::endl;
        gribInfo = dataSource.updateInfo();

    } else {
        // read from json
        std::cout << "Read from " << jsonFileName_ << std::endl;
        gribInfo.fromJSONFile(jsonFileName_);
    }
    
    ASSERT(gribInfo.ready());

    if (doQuery_) {
        if (doRange_){
            std::vector<double> v = gribInfo.extractAtIndexRangeOfRanges(dataSource, rangesVector);
            std::cout << "Value: " << v << std::endl;
        }
        else{
            size_t index = std::stoi(args(2));
            std::cout << "Query index " << index << " in " << gribFileName_ << std::endl;
            double v = gribInfo.extractAtIndex(dataSource, index);
            std::cout << "Value: " << v << std::endl;
        }
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    GribJump tool(argc,argv);
    return tool.start();
}
