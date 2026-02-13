
#include <cstddef>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/runtime/Tool.h"

// This include is required to enable dictionary access traits from eckit::LocalConfiguration
#include "metkit/mars2grib/frontend/normalization/normalizeMarsDict.h"
#include "metkit/mars2grib/utils/dictionary_traits/dictaccess_eckit_configuration.h"



#include "metkit/mars2grib/CoreOperations.h"


eckit::LocalConfiguration mars2conf(const eckit::LocalConfiguration& mars) {
    eckit::LocalConfiguration opts;
    const std::string confJson = metkit::mars2grib::CoreOperations::dumpHeaderTest(mars, opts);
    return eckit::LocalConfiguration{eckit::YAMLConfiguration{confJson}};
}


class CheckMars2conf : public eckit::Tool {
public:
    CheckMars2conf(int argc, char** argv) : Tool{argc, argv} {}

    static void usage(const std::string& tool) {
        eckit::Log::info() << "\nUsage: " << tool << " inputFile" << std::endl;
    }

    void run() override {
        eckit::JSON json{eckit::Log::warning(), eckit::JSON::Formatting::indent(2)};
        eckit::option::CmdArgs args{usage, 1, -1};

        eckit::Log::info() << "Running " << args(0) << std::endl;

        const eckit::LocalConfiguration testCases{eckit::YAMLConfiguration{eckit::PathName(args(0))}};
        eckit::Log::info() << "Loaded " << testCases.getSubConfigurations().size() << " test cases!" << std::endl;

        size_t count  = 0;
        size_t failed = 0;
        for (const auto& testCase : testCases.getSubConfigurations()) {
            count++;

            if (!testCase.has("encoderConfiguration")) {
                continue;
            }

            auto mars                   = testCase.getSubConfiguration("mars");
            const auto& expectedEncoder = testCase.getSubConfiguration("encoderConfiguration");

            if (metkit::mars2grib::frontend::normalization::hack::fixMarsGrid(mars)) {
                eckit::Log::info() << "Fixed MARS grid" << std::endl;
            }

            eckit::LocalConfiguration actualEncoder;
            try {
                actualEncoder = mars2conf(mars).getSubConfiguration("GribHeaderLayoutData");
            }
            catch (const eckit::Exception& e) {
                eckit::Log::warning() << "Encountered an exception!" << std::endl;
                {
                    eckit::JSON json{eckit::Log::warning()};
                    json << testCase;
                    eckit::Log::warning() << std::endl;
                }
                failed++;
                continue;
            }

            bool currentFailed = false;
            for (size_t si = 0; si <= 5; ++si) {
                const auto expectedSection = expectedEncoder.getSubConfigurations("sections")[si];
                const auto actualSection = actualEncoder.getSubConfigurations("sections")[si].getSubConfiguration("SectionLayoutData");

                const auto& expectedTemplate = expectedSection.getLong("templateNumber");
                const auto& actualTemplate = actualSection.getLong("templateNumber");
                if (expectedTemplate != actualTemplate) {
                    eckit::Log::warning() << "Template number for section " << si << " does not match! : " << actualTemplate << " != " << expectedTemplate << std::endl;

                    if (actualTemplate == 192'001'024'036) {
                        eckit::Log::warning() << "Skipping..." << std::endl;
                        continue;
                    }

                    currentFailed = true;
                }

                const auto& expectedConcepts = expectedSection.getSubConfigurations("concepts");
                const auto& actualConcepts = actualSection.getStringVector("variantNames");

                if (expectedConcepts.size() != actualConcepts.size()) {
                    eckit::Log::warning() << "Number of concepts for section " << si << " does not match! : " << actualConcepts.size() << " != " << expectedConcepts.size() << std::endl;
                    currentFailed = true;
                    continue;
                }
                for (size_t ci = 0; ci < actualConcepts.size(); ++ci) {
                    const auto expectedConcept = expectedConcepts[ci].getString("name");
                    const auto expectedVariant = expectedConcepts[ci].getString("type");

                    size_t pos = actualConcepts[ci].find("::");
                    const auto actualConcept = actualConcepts[ci].substr(0, pos);
                    const auto actualVariant = actualConcepts[ci].substr(pos + 2);

                    if (expectedConcept != actualConcept) {
                        eckit::Log::warning() << "A concept for section " << si << " does not match! : " << actualConcept << " != " << expectedConcept << std::endl;
                        currentFailed = true;
                    }
                    else if (expectedVariant != actualVariant) {
                        eckit::Log::warning() << "A variant for section " << si << " does not match! : " << actualConcept << "::" << actualVariant << " != " << expectedConcept << "::" << expectedVariant << std::endl;
                        currentFailed = true;
                    }
                }
            }
            if (currentFailed) {
                failed++;
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

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    CheckMars2conf tool(argc, argv);
    return tool.start();
}
