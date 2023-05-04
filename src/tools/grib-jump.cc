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

    virtual int minimumPositionalArguments() const { return 1; }
    virtual void execute(const eckit::option::CmdArgs& args);
    virtual void init(const eckit::option::CmdArgs& args);
    virtual void usage(const std::string& tool) const;
    GribInfo extract();
    double query(GribInfo, size_t);
    std::vector<double> queryRangeNaive(GribInfo gribInfo, size_t start_i, size_t end_i);
    std::vector<double> queryRange(GribInfo gribInfo, size_t start_i, size_t end_i);
    void test();

private: // members
    bool doExtract_ = false;
    bool doQuery_ = false;
    bool doRange_ = false;
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
    // XXX: better arg parsing
    if (args(0) == "-x" && args.count() == 2){
        doExtract_ = true;
    }
    else if (args(0) == "-q" && args.count() == 3){
        doQuery_ = true;
    }
    else if (args(0) == "-qr" && args.count() == 4){
        doQuery_ = true;
        doRange_ = true; // XXX hacky for now
    }
    else if (args(0) == "-test" && args.count() == 1){
        // XXX
        test();
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
        if (doRange_){
            size_t start_i = std::stoi(args(2));
            size_t end_i = std::stoi(args(3));
            std::cout << "Query index range [" << start_i << ", " << end_i << ") in " << gribFileName_ << std::endl;
            std::vector<double> v = queryRangeNaive(gribInfo, start_i, end_i);
            std::cout << "Value: " << v << std::endl;
        }
        else{
            size_t index = std::stoi(args(2));
            std::cout << "Query index " << index << " in " << gribFileName_ << std::endl;
            double v = query(gribInfo, index);
            std::cout << "Value: " << v << std::endl;
        }
    }
}

GribInfo GribJump::extract() {
    GribHandleData dataSource(gribFileName_);
    GribInfo gribInfo = dataSource.updateInfo();
    return gribInfo;
}

double GribJump::query(GribInfo gribInfo, size_t index) {
    // Given an index, query the grib (with the aid of the json) to find and print the index-th
    // double/float value
    GribHandleData dataSource(gribFileName_);
    double v = gribInfo.extractAtIndex(dataSource, index);
    return v;
}
std::vector<double> GribJump::queryRangeNaive(GribInfo gribInfo, size_t start_i, size_t end_i) {
    // Given an index, query the grib (with the aid of the json) to find and print the index-th
    // double/float value
    GribHandleData dataSource(gribFileName_);
    std::vector<double> v = gribInfo.extractAtIndexRangeNaive(dataSource, start_i, end_i);
    return v;
}
std::vector<double> GribJump::queryRange(GribInfo gribInfo, size_t start_i, size_t end_i) {
    // Given an index, query the grib (with the aid of the json) to find and print the index-th
    // double/float value
    GribHandleData dataSource(gribFileName_);
    std::vector<double> v = gribInfo.extractAtIndexRange(dataSource, start_i, end_i);
    return v;
}

void GribJump::test() {
    // simple test
    GribInfo gribInfo;
    gribFileName_ = "sl_test.grib";
    jsonFileName_ = "sl_test.grib.json";

    // write to json
    gribInfo = extract();

    // read back from json
    std::cout << "Read from " << jsonFileName_ << std::endl;
    gribInfo.fromJSONFile(jsonFileName_);

    // check the json is as expected.
    std::stringstream s;
    eckit::JSON json(s);
    gribInfo.toJSON(json);
    std::string test_str = "{\"binaryScaleFactor\":-13,\"decimalScaleFactor\":0,\"bitsPerValue\":16,"
    "\"referenceValue\":31.11083984375,\"offsetBeforeData\":121,\"numberOfDataPoints\":90,\"numberOfValues\":56,"
    "\"offsetBeforeBitmap\":98,\"sphericalHarmonics\":0,\"binaryMultiplier\":0.0001220703125,\"decimalMultiplier\":1}";
    ASSERT(s.str() == test_str);
    std::cout << "extract/read json test passed" << std::endl;

    // check that the indices read from grib are as expected.
    // -- input with bitmask --
    unsigned long numberOfDataPoints = 90;
    double expected_v[] = {
        31.11083984375, 31.11083984375, 31.11083984375, 31.11083984375, 31.11083984375,
        31.11083984375, 31.11083984375, 31.11083984375, 31.11083984375, 31.11083984375,
        31.11083984375, 31.11083984375, 31.11083984375, 31.11083984375, 31.11083984375,
        31.11083984375, 31.11083984375, 31.11083984375, 9999, 9999,
        9999, 9999, 9999, 9999, 9999,
        33.9698486328125, 33.070068359375, 33.5550537109375, 33.3505859375, 32.7520751953125,
        9999, 9999, 9999, 31.3387451171875, 36.007568359375,
        35.7113037109375, 33.92529296875, 9999, 9999, 35.53125,
        34.48876953125, 9999, 9999, 34.6346435546875, 34.8411865234375,
        35.33154296875, 35.1544189453125, 35.32861328125, 35.18359375, 35.0867919921875,
        9999, 9999, 35.44287109375, 35.8751220703125, 34.117919921875,
        33.8076171875, 33.765869140625, 33.9698486328125, 33.9305419921875, 34.5517578125,
        34.6268310546875, 35.037841796875, 34.8756103515625, 34.539794921875, 34.5748291015625,
        34.3450927734375, 34.1849365234375, 34.124755859375, 33.824951171875, 34.025634765625,
        34.5318603515625, 34.437255859375, 9999, 9999, 9999,
        9999, 9999, 9999, 9999, 9999,
        9999, 9999, 9999, 9999, 9999,
        9999, 9999, 9999, 9999, 9999
    };
    ASSERT(gribInfo.ready());
    for (size_t index = 0; index < numberOfDataPoints; index++) {
        double v = query(gribInfo, index);
        double delta = std::abs(v - expected_v[index]);
        ASSERT(delta < 1e-15);
    }
    std::cout << "query indices test passed (with bitmask)" << std::endl;

    // test range query
    size_t i_start = 10;
    size_t i_end = 80;
    std::vector<double> values_naive = queryRangeNaive(gribInfo, i_start, i_end);
    ASSERT(values_naive.size() == (i_end - i_start));
    for (size_t i = 0; i < i_end - i_start; i++) {
        double delta = std::abs(values_naive[i] - expected_v[i_start + i]);
        ASSERT(delta < 1e-12);
    }
    std::cout << "query naive range test passed (with bitmask)" << std::endl;


    // test range query
    std::vector<double> values = queryRange(gribInfo, i_start, i_end);
    ASSERT(values.size() == (i_end - i_start));
    for (size_t i = 0; i < i_end - i_start; i++) {
        double delta = std::abs(values[i] - expected_v[i_start + i]);
        ASSERT(delta < 1e-12);
    }
    std::cout << "query smarter range test passed (with bitmask)" << std::endl;

    // -- input without bitmask --

    gribFileName_ = "no_mask.grib";
    jsonFileName_ = "no_mask.grib.json";
    GribInfo gribInfo2 = extract();
//
    double expected_v2[] = {
        245.789428710938, 245.789428710938, 245.789428710938, 245.789428710938, 245.789428710938,
        245.789428710938, 245.789428710938, 245.789428710938, 253.002319335938, 253.116088867188,
        252.827026367188, 252.722534179688, 252.907104492188, 252.855834960938, 252.554077148438,
        252.596557617188, 265.281127929688, 266.042846679688, 266.145874023438, 265.752319335938,
        265.715209960938, 265.834350585938, 265.416870117188, 264.930053710938, 252.105346679688,
        252.270874023438, 252.200561523438, 251.764526367188, 251.864135742188, 252.612670898438,
        252.925170898438, 252.447143554688, 244.540405273438, 244.540405273438, 244.540405273438,
        244.540405273438, 244.540405273438, 244.540405273438, 244.540405273438, 244.540405273438
    };
    numberOfDataPoints = 40;
    for (size_t index = 0; index < numberOfDataPoints; index++) {
        double v = query(gribInfo2, index);
        double delta = std::abs(v - expected_v2[index]);
        ASSERT(delta < 1e-12);
    }
    std::cout << "query indices test passed (without bitmask)" << std::endl;

    // test range query
    i_start = 5;
    i_end = 35;
    std::vector<double> values_naive2 = queryRangeNaive(gribInfo2, i_start, i_end);
    ASSERT(values_naive2.size() == (i_end - i_start));
    for (size_t i = 0; i < i_end - i_start; i++) {
        double delta = std::abs(values_naive2[i] - expected_v2[i_start + i]);
        ASSERT(delta < 1e-12);
    }
    std::cout << "query naive range test passed (without bitmask)" << std::endl;

    // test range query
    std::vector<double> values_2 = queryRange(gribInfo2, i_start, i_end);
    ASSERT(values_2.size() == (i_end - i_start));
    for (size_t i = 0; i < i_end - i_start; i++) {
        double delta = std::abs(values_2[i] - expected_v2[i_start + i]);
        ASSERT(delta < 1e-12);
    }
    std::cout << "query smarter range test passed (without bitmask)" << std::endl;

    exit(0);
}
//----------------------------------------------------------------------------------------------------------------------

int main(int argc,char **argv)
{
    GribJump tool(argc,argv);
    return tool.start();
}
