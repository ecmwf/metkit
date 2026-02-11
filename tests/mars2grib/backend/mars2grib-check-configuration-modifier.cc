
/*
 * (C) Copyright 2025- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>
#include <unordered_set>
#include <vector>

#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/LocalPathName.h"
#include "eckit/filesystem/PathName.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_codes_handle.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"

#include "metkit/mars2grib/backend/SpecializedEncoder.h"
#include "metkit/mars2grib/utils/mars2gribExceptions.h"

int main(int argc, char** argv) {

    using metkit::mars2grib::backend::SpecializedEncoder;
    using metkit::mars2grib::backend::config::EncoderCfg;
    using metkit::mars2grib::backend::config::makeEncoderCallbacks;
    using metkit::mars2grib::backend::config::makeEncoderConfiguration;
    using metkit::mars2grib::backend::config::printEncoderConfiguration;
    using metkit::mars2grib::utils::exceptions::printExceptionStack;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <test-cases-file>" << std::endl;
        return 1;
    }

    std::string testCasesFile = argv[1];
    std::cout << "Loading test cases from file: " << testCasesFile << std::endl;
    const eckit::LocalConfiguration testCases{eckit::YAMLConfiguration{eckit::PathName(testCasesFile)}};
    const std::vector<eckit::LocalConfiguration> testCasesList = testCases.getSubConfigurations("test-cases");
    eckit::Log::info() << "Loaded " << testCasesList.size() << " test cases!" << std::endl;

    size_t count  = 0;
    size_t failed = 0;
    for (const auto& testCase : testCasesList) {
        const auto cfg = testCase.getSubConfiguration("encoder");
        count++;
        try {
            // EncoderCfg encoderCfg = parseEncoderCfg(cfg);
            auto encoderCfg = makeEncoderConfiguration(cfg);
            printEncoderConfiguration(encoderCfg);

            auto Callbacks =
                makeEncoderCallbacks<eckit::LocalConfiguration, eckit::LocalConfiguration, eckit::LocalConfiguration,
                                     eckit::LocalConfiguration, metkit::codes::CodesHandle>(encoderCfg);

            auto xxx =
                SpecializedEncoder<eckit::LocalConfiguration, eckit::LocalConfiguration, eckit::LocalConfiguration,
                                   eckit::LocalConfiguration, metkit::codes::CodesHandle>(encoderCfg);

            auto yyy =
                SpecializedEncoder<eckit::LocalConfiguration, eckit::LocalConfiguration, eckit::LocalConfiguration,
                                   eckit::LocalConfiguration, metkit::codes::CodesHandle>(cfg);
        }
        catch (const std::exception& e) {
            // failed++;
            // std::cout << "Test case " << count << " FAILED: " << e.what() << std::endl;
            printExceptionStack(e, std::cerr);
            return 1;
        }
    }

    std::cout << "Tests run: " << count << ", Failures: " << failed << std::endl;

    return 0;
}
