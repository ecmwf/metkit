
#include "eckit/io/DataHandle.h"
#include "eckit/io/FileDescHandle.h"
#include "eckit/io/FileHandle.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars/MarsParser.h"
#include "metkit/tool/MetkitTool.h"

#include <fstream>

#include <unistd.h>


using namespace metkit;
using namespace metkit::mars;
using namespace eckit;
using namespace eckit::option;

//----------------------------------------------------------------------------------------------------------------------

class MarsArchiveScript : public MetkitTool {

    using OverridesDict = std::map<std::string, std::map<std::string, std::vector<std::string>>>;

public: // methods

    MarsArchiveScript(int argc, char** argv);
    ~MarsArchiveScript() override = default;

private: // methods

    void init(const CmdArgs& args) override;
    void execute(const CmdArgs& args) override;
    void usage(const std::string& tool) const override;

    OverridesDict extractOverrides(MarsRequest& request);

    template <typename ...Args>
    void setOverrides(MarsRequest& rq, const OverridesDict& overrides, const char* overrideName, Args... more);

    void setOverrides(MarsRequest& rq, const OverridesDict& overrides);

    std::string srcfile(int cnt) const;
    std::string arcfile(int cnt) const;
    std::string cmpfile(int cnt) const;

private: // members

    std::string infile_;
    std::string outfile_;
    std::string tempPrefix_;

    bool obs_;
    bool compare_;

    // Output environment variables

    std::string preObs_;
    std::string retrieveMars_;
    std::string archiveMars_;
    std::string compareMars_;

    std::string unblockCommand_;

    std::string retrieveComplete_;
    std::string archiveComplete_;
    std::string compareComplete_;
    std::string compareCommand_;
};

//----------------------------------------------------------------------------------------------------------------------

MarsArchiveScript::MarsArchiveScript(int argc, char** argv) :
    MetkitTool(argc, argv),
    tempPrefix_("mars"),
    obs_(false),
    compare_(false),
    preObs_("PREOBS"),
    retrieveMars_("RETRIEVE_MARS"),
    archiveMars_("ARCHIVE_MARS"),
    compareMars_("COMPARE_MARS"),
    unblockCommand_("UNBLOCK"),
    retrieveComplete_("RETRIEVE_COMPLETE"),
    archiveComplete_("ARCHIVE_COMPLETE"),
    compareComplete_("COMPARE_COMPLETE"),
    compareCommand_("COMPARE") {

    options_.push_back(
        new SimpleOption<std::string>("out", "Output filename (defaults output to stdout)"));

    options_.push_back(
        new SimpleOption<std::string>("in", "Input filename (defaults input to stdin)"));

    options_.push_back(
            new SimpleOption<bool>("obs", "Handle observations rather than output GRIBs"));

    options_.push_back(
            new SimpleOption<bool>("compare", "Re-retrieve and compare results"));

    options_.push_back(
        new SimpleOption<bool>("legacy", "Use legacy environment variable names for old suite compatability"));

    options_.push_back(
        new SimpleOption<std::string>("prefix", "Prefix for the temporary files. Typically \"mars\""));
}

void MarsArchiveScript::usage(const std::string& tool) const {

    Log::info() << "Usage: " << tool << " [options]" << eckit::newl
                << eckit::newl
                << "Note: The output of this tool assumes that we are running in an ecflow suite with" << eckit::newl
                << "      failure trapping enabled, as well as the following environment variables:" << eckit::newl
                << "      RETRIEVE_MARS, ARCHIVE_MARS, COMPARE_MARS, UNBLOCK, RETRIEVE_COMPLETE" << eckit::newl
                << "      ARCHIVE_COMPLETE, COMPARE_COMPLETE" << eckit::newl
                << eckit::newl

                << "Examples:" << eckit::newl
                << "=========" << eckit::newl
                << std::endl;
}

void MarsArchiveScript::init(const CmdArgs& args) {

    infile_ = args.getString("in", infile_);
    outfile_ = args.getString("out", outfile_);

    compare_ = args.getBool("compare", compare_);
    tempPrefix_ = args.getString("prefix", tempPrefix_);
    obs_ = args.getBool("obs", obs_);

    if (args.getBool("legacy", false)) {
        retrieveMars_ = "MARS_FROM_FDB";
        archiveMars_ = "MARS_TO_IBM";
        compareMars_ = "MARS_FROM_IBM";
        unblockCommand_ = "UNBLOCK";
        retrieveComplete_ = "FDB_COMPLETE";
        archiveComplete_ = "ARC_COMPLETE";
        compareComplete_ = "CMP_COMPLETE";
    }

    if (obs_) compareCommand_ = "COMPOBS";
}

std::string MarsArchiveScript::srcfile(int cnt) const {
    return tempPrefix_ + ".source." + translate<std::string>(cnt);
}

std::string MarsArchiveScript::arcfile(int cnt) const {
    return tempPrefix_ + ".archive." + translate<std::string>(cnt);
}

std::string MarsArchiveScript::cmpfile(int cnt) const {
    return tempPrefix_ + ".compare." + translate<std::string>(cnt);
}

MarsArchiveScript::OverridesDict MarsArchiveScript::extractOverrides(MarsRequest& request) {

    OverridesDict ret;

    std::vector<std::string> keys;
    request.getParams(keys);

    for (const auto& k : keys) {
        int pos = k.find('@');
        if (pos != std::string::npos) {
            std::vector<std::string> vals;
            request.getValues(k, vals);
            ret[k.substr(0, pos)][k.substr(pos+1)] = vals;
            request.erase(k);
        }
    }

    return ret;
}

template <typename ...Args>
void MarsArchiveScript::setOverrides(MarsRequest& rq, const OverridesDict& overrides, const char* overrideName, Args... more) {

    auto namedset = overrides.find(overrideName);
    if (namedset != overrides.end()) {
        for (const auto& kv : namedset->second) {
            rq.setValue(kv.first, kv.second);
        }
    }

    setOverrides(rq, overrides, more...);
}

void MarsArchiveScript::setOverrides(MarsRequest& rq, const OverridesDict& overrides) {}

void MarsArchiveScript::execute(const CmdArgs& args) {

    // Input from file, or from stdin?

    std::unique_ptr<std::ifstream> in_file;
    if (!infile_.empty()) {
        in_file = std::make_unique<std::ifstream>(infile_.c_str(), std::ios::in | std::ios::binary);
        in_file->exceptions(std::ios::badbit);
    }
    std::istream& in(in_file ? *in_file : std::cin);

    // Output to file, or to stdout?

    std::unique_ptr<std::ofstream> out_file;
    if (!outfile_.empty()) {
        out_file = std::make_unique<std::ofstream>(outfile_.c_str(), std::ios::out | std::ios::binary);
        out_file->exceptions(std::ios::badbit);
    }
    std::ostream& out(out_file ? *out_file : std::cout);

    // Parse the input request

    MarsParser parser(in);
    auto requests = parser.parse();

    std::vector<OverridesDict> overrides;
    for (auto& rq : requests) {
        overrides.emplace_back(extractOverrides(rq));
    }

    // 1. Retrieve the source data (typically from the FDB)
    // n.b. loop over non-const, non-reference requests --> mutable copy in the loops

    int cnt = 0;

    if (obs_) {

        for (cnt = 0; cnt < requests.size(); ++cnt) {
            out << "$" << preObs_ << " $header." << translate<std::string>(cnt+1)
                                  << " $data." << translate<std::string>(cnt+1)
                                  << " " << arcfile(cnt) << "\n\n";
        }

    } else {

        out << "$" << retrieveMars_ << " << @\n\n";

        cnt = 0;
        for (auto rq: requests) {
            rq.verb("retrieve");
            rq.setValue("target", srcfile(cnt));
            setOverrides(rq, overrides[cnt++], "default", "retrieve", "fdb_retrieve");
            rq.dump(out);
        }

        out << "@\n\n$" << retrieveComplete_ << "\n\n";

        // 1.a) Any intermediate step required. Typically ln -s (historically handle "blocked" data output by older Fortran
        //      based systems)

        for (cnt = 0; cnt < requests.size(); ++cnt) {
            out << "$" << unblockCommand_ << " " << srcfile(cnt) << " " << arcfile(cnt) << "\n\n";
        }
    }

    // 2. Archive the data

    out << "$" << archiveMars_ << " << @\n\n";

    cnt = 0;
    for (auto rq : requests) {
        rq.verb("archive");
        rq.setValue("source", arcfile(cnt));
        setOverrides(rq, overrides[cnt++], "default", "archive");
        rq.dump(out);
    }

    out << "@\n\n$" << archiveComplete_ << "\n\n";

    // 3. Comparison of re-retrieved data

    if (compare_) {

        out << "$" << compareMars_ << " << @\n\n";

        cnt = 0;
        for (MarsParsedRequest rq : requests) {
            rq.verb("retrieve");
            rq.setValue("target", cmpfile(cnt));
            setOverrides(rq, overrides[cnt++], "default", "compare", "ibm_retrieve");
            rq.dump(out);
        }

        out << "@\n\n";

        for (cnt = 0; cnt < requests.size(); ++cnt) {
            out << "$" << compareCommand_ << " " << arcfile(cnt) << " " << cmpfile(cnt) << "\n\n";
        }

        out << "$" << compareComplete_ << "\n";
    }

    out << std::endl;
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    MarsArchiveScript tool(argc, argv);
    return tool.start();
}