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
#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/LocalPathName.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/CodeLocation.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/runtime/Tool.h"
#include "metkit/mars2grib/frontend/encoderConfig.h"

bool compareLocalConfig(const eckit::LocalConfiguration& lhs, const eckit::LocalConfiguration& rhs) {
    auto lhsKeysVector = lhs.keys();
    auto rhsKeysVector = rhs.keys();
    std::unordered_set<std::string> lhsKeys{lhsKeysVector.begin(), lhsKeysVector.end()},
        rhsKeys{rhsKeysVector.begin(), rhsKeysVector.end()};

    if (lhsKeys.size() != rhsKeys.size()) {
        eckit::Log::warning() << "Different number of keys: " << lhsKeysVector << " vs " << rhsKeysVector << std::endl;
        return false;
    }
    for (const auto& k : lhsKeys) {
        if (auto search = rhsKeys.find(k); search == rhsKeys.end()) {
            eckit::Log::warning() << "Keys " << k << " not given in rhs: " << rhsKeysVector << std::endl;
            return false;
        }
    }

    for (const auto& k : lhsKeys) {
        if (lhs.isString(k)) {
            if (!rhs.isString(k)) {
                eckit::Log::warning() << "Keys " << k << " is String but rhs is not " << rhs << std::endl;
                return false;
            }
            auto lhsVal = lhs.getString(k);
            auto rhsVal = rhs.getString(k);
            if (lhsVal != rhsVal) {
                eckit::Log::warning() << "Values for key " << k << " differ: " << lhsVal << " != " << rhsVal
                                      << std::endl;
                return false;
            }
        }
        else if (lhs.isIntegral(k)) {
            if (!rhs.isIntegral(k)) {
                eckit::Log::warning() << "Keys " << k << " is Integral but rhs is not " << rhs << std::endl;
                return false;
            }
            auto lhsVal = lhs.getInt64(k);
            auto rhsVal = rhs.getInt64(k);
            if (lhsVal != rhsVal) {
                eckit::Log::warning() << "Values for key " << k << " differ: " << lhsVal << " != " << rhsVal
                                      << std::endl;
                return false;
            }
        }
        else if (lhs.isSubConfiguration(k)) {
            if (!rhs.isSubConfiguration(k)) {
                eckit::Log::warning() << "Keys " << k << " is subConfiguration but rhs is not " << rhs << std::endl;
                return false;
            }
            if (!compareLocalConfig(rhs.getSubConfiguration(k), lhs.getSubConfiguration(k))) {
                return false;
            }
        }
        else {
            eckit::Log::warning() << "Unhandled type for key " << k << " - lhs: " << lhs << " rhs: " << rhs
                                  << std::endl;
            return false;
        }
    }
    return true;
}

class CompareMarsToEncoder : public eckit::Tool {
public:

    CompareMarsToEncoder(int argc, char** argv) : eckit::Tool{argc, argv} {}

    static void usage(const std::string& tool) {
        eckit::Log::info() << "\nUsage: " << tool << " inputFile" << std::endl;
    }

    void run() override {
        eckit::JSON json{eckit::Log::warning(), eckit::JSON::Formatting::indent(2)};
        eckit::option::CmdArgs args{usage, 1, -1};

        const eckit::LocalConfiguration testCases{eckit::YAMLConfiguration{eckit::PathName(args(0))}};
        eckit::Log::info() << "Loaded " << testCases.getSubConfigurations().size() << " test cases!" << std::endl;

        size_t count  = 0;
        size_t failed = 0;
        for (const auto& testCase : testCases.getSubConfigurations()) {
            count++;

            const auto& mars            = testCase.getSubConfiguration("mars");
            const auto& expectedEncoder = testCase.getSubConfiguration("conf");

            eckit::LocalConfiguration actualEncoder;
            try {
                actualEncoder = metkit::mars2grib::frontend::buildEncoderConfig(mars);
            }
            catch (const eckit::Exception& e) {
                eckit::Log::warning() << "Encountered an exception!" << std::endl;
                failed++;
                continue;
            }

            if (!compareLocalConfig(expectedEncoder, actualEncoder)) {
                failed++;
                eckit::Log::warning() << "==================== FAILURE! ====================" << std::endl;
                {
                    eckit::JSON json{eckit::Log::warning(), eckit::JSON::Formatting::indent(2)};
                    eckit::Log::warning() << "{" << std::endl << "\"mars\" : ";
                    json << mars;
                    eckit::Log::warning() << "," << std::endl << std::endl;
                }
                {
                    eckit::JSON json{eckit::Log::warning(), eckit::JSON::Formatting::indent(2)};
                    eckit::Log::warning() << "\"expected-encoder\" : ";
                    json << expectedEncoder;
                    eckit::Log::warning() << "," << std::endl << std::endl;
                }
                {
                    eckit::JSON json{eckit::Log::warning(), eckit::JSON::Formatting::indent(2)};
                    eckit::Log::warning() << "\"actual-encoder\" : ";
                    json << actualEncoder;
                    eckit::Log::warning() << std::endl << "}" << std::endl;
                }
                eckit::Log::warning() << "\n==================================================" << std::endl;
            }
        }

        std::ostringstream oss;
        oss << "Failed " << failed << " cases failed out of " << count << std::endl;
        eckit::Log::error() << oss.str();
        if (failed != 0) {
            throw eckit::Exception(oss.str(), Here());
        }
    }
};

int main(int argc, char** argv) {
    CompareMarsToEncoder tool{argc, argv};
    return tool.start();
}
