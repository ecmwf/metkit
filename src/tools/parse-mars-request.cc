/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <algorithm>
#include <fstream>

#include "eckit/io/Buffer.h"
#include "eckit/io/Offset.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/tool/MetkitTool.h"


using namespace metkit;
using namespace metkit::mars;
using namespace eckit;
using namespace eckit::option;

//----------------------------------------------------------------------------------------------------------------------

class ParseRequest : public MetkitTool {
public:

    ParseRequest(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new SimpleOption<bool>("json", "Format request in json, default = false"));
        options_.push_back(new SimpleOption<bool>("compact", "Compact output, default = false"));
    }

    virtual ~ParseRequest() {}

private:  // methods

    int minimumPositionalArguments() const { return 1; }

    void process(const eckit::PathName& path);

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual void init(const CmdArgs& args);

    virtual void usage(const std::string& tool) const;

private:  // members

    bool json_    = false;
    bool compact_ = false;
};

//----------------------------------------------------------------------------------------------------------------------

void ParseRequest::execute(const eckit::option::CmdArgs& args) {
    for (size_t i = 0; i < args.count(); i++) {
        process(args(i));
    }
}

void ParseRequest::init(const CmdArgs& args) {
    args.get("json", json_);
    args.get("compact", compact_);
    args.get("porcelain", porcelain_);
    if (porcelain_)
        compact_ = true;
}

void ParseRequest::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] [request1] [request2] ..." << std::endl << std::endl;

    Log::info() << "Examples:" << std::endl
                << "=========" << std::endl
                << std::endl
                << tool << " --json mars1.req mars2.req" << std::endl
                << tool << " --porcelain folderOfRequests" << std::endl
                << std::endl;
}

void ParseRequest::process(const eckit::PathName& path) {

    if (path.isDir()) {
        std::vector<eckit::PathName> files;
        std::vector<eckit::PathName> directories;

        path.children(files, directories);

        std::sort(files.begin(), files.end());
        std::sort(directories.begin(), directories.end());

        for (std::vector<eckit::PathName>::const_iterator j = files.begin(); j != files.end(); ++j) {
            process(*j);
        }

        for (std::vector<eckit::PathName>::const_iterator j = directories.begin(); j != directories.end(); ++j) {
            process(*j);
        }
        return;
    }


    if (!porcelain_) {
        std::cout << "==========> Parsing : " << path << std::endl;
    }

    std::ifstream in(path.asString().c_str());
    MarsParser parser(in);

    bool inherit = true;
    MarsExpansion expand(inherit);

    auto p = parser.parse();
    if (!porcelain_) {
        for (auto j = p.begin(); j != p.end(); ++j) {
            if (compact_) {
                j->dump(std::cout, "", "");
                std::cout << std::endl;
            }
            else {
                j->dump(std::cout);
            }
        }

        std::cout << "----------> Expanding ... " << std::endl;
    }


    std::vector<MarsRequest> v = expand.expand(p);

    for (std::vector<MarsRequest>::const_iterator j = v.begin(); j != v.end(); ++j) {
        if (json_) {
            if (compact_) {
                eckit::JSON jsonOut(std::cout);
                j->json(jsonOut);
            }
            else {
                eckit::JSON jsonOut(std::cout, eckit::JSON::Formatting(eckit::JSON::Formatting::BitFlags::INDENT_DICT));
                j->json(jsonOut);
            }
            std::cout << std::endl;
        }
        else {
            if (compact_) {
                j->dump(std::cout, "", "");
                std::cout << std::endl;
            }
            else {
                j->dump(std::cout);
            }
        }
    }

    class Print : public FlattenCallback {
        virtual void operator()(const MarsRequest& request) { std::cout << request << std::endl; }
    };


    Print cb;

    // for (std::vector<MarsRequest>::const_iterator j = v.begin(); j != v.end(); ++j) {
    //     expand.flatten(*j, cb, filter);
    // }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    ParseRequest tool(argc, argv);
    return tool.start();
}
