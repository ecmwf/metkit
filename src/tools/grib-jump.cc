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

    GribJump(int argc, char **argv) : metkit::MetkitTool(argc, argv) {}

private: // methods

    virtual int minimumPositionalArguments() const { return 2; }
    virtual void execute(const eckit::option::CmdArgs& args);
    virtual void init(const eckit::option::CmdArgs& args);
    virtual void usage(const std::string& tool) const;
    GribInfo extract();
    void query(GribInfo, size_t);

private: // members
    bool doExtract_ = false;
    bool doQuery_ = false;
    eckit::PathName gribFileName_;
    eckit::PathName jsonFileName_;
};

void GribJump::usage(const std::string &tool) const {
    eckit::Log::info() << std::endl
                        << "Usage: " << tool << " <mode> <path/to/grib> "
                        << std::endl;

    eckit::Log::info() << "Examples:" << std::endl
                        << "=========" << std::endl
                        << std::endl
                        << "e.g. Process and relevant extract info from data.grib to data.json:" << std::endl
                        << tool << " -x data.grib" << std::endl
                        << std::endl
                        << "e.g. Query index 123 in data.grib, assuming data.json exists:" << std::endl
                        << tool << " -q data.grib 123" << std::endl
                        << std::endl;
}

void GribJump::init(const eckit::option::CmdArgs& args) {
    if (args(0) == "-x" && args.count() == 2){
        doExtract_ = true;
    }
    else if (args(0) == "-q" && args.count() == 3){
        doQuery_ = true;
    }
    else {
        std::cout << "Invalid mode?" << std::endl;
        usage(args.tool());
        return;
    }

    gribFileName_ = args(1);
    ASSERT(gribFileName_.exists());

    jsonFileName_ = gribFileName_.baseName() + ".json";
    doExtract_ |= !jsonFileName_.exists(); // if json doesn't exist, extract before query
    
}

void GribJump::execute(const eckit::option::CmdArgs& args) {
    GribInfo gribInfo;
    if (doExtract_) {
        std::cout << "Extract from " << args(1) << std::endl;
        gribInfo = extract();
    } else {
        // read from json
        std::cout << "Read from " << jsonFileName_ << std::endl;
        gribInfo.fromJSONFile(jsonFileName_);
    }
    
    ASSERT(gribInfo.ready());
    if (doQuery_) {
        size_t index = std::stoi(args(2));
        std::cout << "Query index " << index << " in " << args(1) << std::endl;
        query(gribInfo, index);
    }
}

GribInfo GribJump::extract() {
    GribHandleData dataSource(gribFileName_);
    GribInfo gribInfo = dataSource.updateInfo();
    return gribInfo;
}

void GribJump::query(GribInfo gribInfo, size_t index) {
    // Given an index, query the grib (with the aid of the json) to find and print the index-th
    // double/float value
    GribHandleData dataSource(gribFileName_);
    double v = gribInfo.extractAtIndex(dataSource, index);
    std::cout << "v = " << v << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    GribJump tool(argc,argv);
    return tool.start();
}