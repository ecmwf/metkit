
#include <unordered_set>
#include "eckit/config/LocalConfiguration.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/runtime/Tool.h"



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

        const eckit::LocalConfiguration testCases{eckit::YAMLConfiguration{eckit::PathName(args(0))}};
        eckit::Log::info() << "Loaded " << testCases.getSubConfigurations().size() << " test cases!" << std::endl;

        size_t count  = 0;
        size_t failed = 0;
        for (const auto& testCase : testCases.getSubConfigurations()) {
            if (count >= 1) {
                continue;
            }
            count++;

            const auto& mars            = testCase.getSubConfiguration("mars");
            const auto& expectedEncoder = testCase.getSubConfiguration("encoderConfiguration");

            eckit::LocalConfiguration actualEncoder;
            try {
                actualEncoder = mars2conf(mars);
            }
            catch (const eckit::Exception& e) {
                eckit::Log::warning() << "Encountered an exception!" << std::endl;
                failed++;
                continue;
            }

            for (size_t si = 0; si < 5; ++si) {
                // TODO: Check that template numbers match

                // TODO: Check that concepts match
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
