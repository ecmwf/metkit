/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <exception>
#include <string>
#include <vector>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/filesystem/LocalPathName.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/runtime/Tool.h"
#include "metkit/mars2grib/api/Mars2Grib.h"


bool toBeSkipped(const eckit::PathName& fname, size_t caseNumber) {

    std::vector<size_t> skipCases;

    if (fname.baseName().asString() == "od-enfo.json") {
        // skipCases = {48,49,50,70};
        skipCases = {};
    }
    else if (fname.baseName().asString() == "od-scwv.json") {
        skipCases = {1};
    }
    for (const auto& c : skipCases) {
        if (caseNumber == c) {
            return true;
        }
    }
    return false;
}


class testMars2GribAPI : public eckit::Tool {
public:

    testMars2GribAPI(int argc, char** argv) : eckit::Tool{argc, argv} {}

    static void usage(const std::string& tool) {
        eckit::Log::info() << "\nUsage: " << tool << " inputFile" << std::endl;
    }

    void run() override {
        eckit::option::CmdArgs args{usage, 1, -1};

        const eckit::PathName fname(args(0));
        const eckit::LocalConfiguration testCases{eckit::YAMLConfiguration{fname}};
        eckit::Log::info() << "Loaded " << testCases.getSubConfigurations().size() << " test cases!" << std::endl
                           << std::endl;

        size_t count   = 0;
        size_t failed  = 0;
        size_t skipped = 0;
        for (const auto& testCase : testCases.getSubConfigurations()) {
            count++;

            if (!toBeSkipped(fname, count)) {
                const auto& mars = testCase.getSubConfiguration("mars");
                const auto& misc = testCase.getSubConfiguration("misc");
                const auto& geom = testCase.getSubConfiguration("geom");

                // Skip spherical-harmonics
                if (mars.has("truncation")) {
                    skipped++;
                    continue;
                }

                try {
                    std::vector<double> values(1639680, 0.0);
                    const auto& grib = metkit::mars2grib::Mars2Grib{}.encode(mars, misc, geom, values);
                }
                catch (std::exception e) {
                    eckit::Log::error() << "Failure occured when API was called in test case " << count << std::endl;
                    eckit::JSON json{eckit::Log::error()};
                    json << testCase;
                    eckit::Log::error() << std::endl << std::endl;
                    failed++;
                    break;  // TODO: Remove this to keep running after the first failure!
                }
            }
            else {
                skipped++;
            }
        }

        eckit::Log::error() << "End of test: " << failed << " test cases failed out of " << count << " (skipped "
                            << skipped << " cases)" << std::endl;
        // TODO: Throw exception to make test fail!
    }
};

int main(int argc, char** argv) {
    testMars2GribAPI tool{argc, argv};
    return tool.start();
}
