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
#include <string>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/io/FileHandle.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/Option.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/runtime/Tool.h"
#include "eckit/utils/StringTools.h"

#include "metkit/MetkitTool.h"
#include "metkit/odb/OdbToRequest.h"


using namespace metkit;
using namespace eckit;
using namespace eckit::option;

//----------------------------------------------------------------------------------------------------------------------

class OdbToRequestTool : public MetkitTool {
public:
    OdbToRequestTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(
            new SimpleOption<std::string>("verb", "Verb in the request, default = retrieve"));
        options_.push_back(new SimpleOption<std::string>(
            "database", "add database keyword to requests, default = none"));
        options_.push_back(new SimpleOption<std::string>(
            "source", "add source keyword to requests, default = none"));
        options_.push_back(new SimpleOption<std::string>(
            "target", "add target keyword to requests, default = none"));
        options_.push_back(
            new SimpleOption<bool>("one", "Merge into only one request, default = false"));
        options_.push_back(
            new SimpleOption<bool>("constant", "Only constant columns, default = true"));
        options_.push_back(
            new SimpleOption<bool>("json", "Format request in json, default = false"));
    }

    virtual ~OdbToRequestTool() {}

private:  // methods
    int minimumPositionalArguments() const { return 1; }

    virtual void execute(const eckit::option::CmdArgs& args);

    virtual void init(const CmdArgs& args);

    virtual void usage(const std::string& tool) const;

private:  // members
    std::vector<PathName> paths_;
    std::string verb_     = "retrieve";
    std::string database_ = "";
    std::string source_   = "";
    std::string target_   = "";
    bool one_             = false;
    bool constant_        = true;
    bool json_            = false;
};

//----------------------------------------------------------------------------------------------------------------------

void OdbToRequestTool::init(const CmdArgs& args) {
    args.get("one", one_);
    args.get("constant", constant_);
    args.get("verb", verb_);
    args.get("database", database_);
    args.get("source", source_);
    args.get("target", target_);
    args.get("json", json_);

    if (json_) {
        porcelain_ = true;
    }
}


void OdbToRequestTool::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] [request1] [request2] ..." << std::endl
                << std::endl;

    Log::info() << "Examples:" << std::endl
                << "=========" << std::endl
                << std::endl
                << tool << " --one --verb=retrieve data.odb" << std::endl
                << std::endl;
}

static void toJSON(const std::vector<MarsRequest>& requests) {
    JSON j(Log::info());
    for (auto& r : requests) {
        r.json(j);
    }
    Log::info() << std::endl;
}

static void toStdOut(const std::vector<MarsRequest>& requests) {
    for (auto& r : requests) {
        eckit::Log::info() << r << std::endl;
    }
}

static void addKeyValue(std::vector<MarsRequest>& requests, const std::string& key, const std::string& value) {
    std::transform(requests.begin(), requests.end(), requests.begin(),
                   [key, value](MarsRequest& r) -> MarsRequest {
                       r.setValue(key, value);
                       return r;
                   });
}

void OdbToRequestTool::execute(const eckit::option::CmdArgs& args) {
    PathName inFile(args(0));

    FileHandle dh(inFile);
    dh.openForRead();

    std::vector<MarsRequest> requests = odb::OdbToRequest(verb_, one_, constant_).odbToRequest(dh);


    if (not database_.empty())
        addKeyValue(requests, "database", database_);

    if (StringTools::lower(verb_) == "archive") {
        addKeyValue(requests, "source", inFile);
    }

    if (not source_.empty())
        addKeyValue(requests, "source", source_);

    if (not target_.empty())
        addKeyValue(requests, "target", target_);

    if (json_) {
        toJSON(requests);
    }
    else {
        toStdOut(requests);
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    OdbToRequestTool tool(argc, argv);
    return tool.start();
}
