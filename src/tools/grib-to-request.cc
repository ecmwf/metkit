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
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "eckit/filesystem/PathName.h"
#include "eckit/io/FileHandle.h"
#include "eckit/log/JSON.h"
#include "eckit/log/Log.h"
#include "eckit/message/Message.h"
#include "eckit/message/Reader.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/utils/StringTools.h"

#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/tool/MetkitTool.h"

using namespace metkit;
using namespace metkit::mars;
using namespace eckit;
using namespace eckit::option;

//----------------------------------------------------------------------------------------------------------------------

class MarsRequestSetter : public eckit::message::MetadataGatherer {
public:  // methods

    MarsRequestSetter(MarsRequest& request) : request_(request) {}

    void setValue(const std::string& key, const std::string& value) override { request_.setValue(key, value); }
    void setValue(const std::string& key, long value) override { request_.setValue(key, value); }
    void setValue(const std::string& key, double value) override { request_.setValue(key, value); }

private:  // members

    MarsRequest& request_;
};

//----------------------------------------------------------------------------------------------------------------------

class GribToRequestTool : public MetkitTool {
public:

    GribToRequestTool(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new SimpleOption<std::string>("verb", "Verb in the request, default = retrieve"));
        options_.push_back(
            new SimpleOption<std::string>("database", "add database keyword to requests, default = none"));
        options_.push_back(new SimpleOption<std::string>("source", "add source keyword to requests, default = none"));
        options_.push_back(new SimpleOption<std::string>("target", "add target keyword to requests, default = none"));
        options_.push_back(new SimpleOption<bool>("one",
                                                  "Merge into one request, potentially describing more data "
                                                  "fields than the ones in the input file, default = false"));
        options_.push_back(new SimpleOption<bool>("compact", "Merge into a small number of requests, default = false"));
        options_.push_back(new SimpleOption<bool>("json", "Format request in json, default = false"));
    }

private:  // methods

    int minimumPositionalArguments() const override { return 1; }

    void execute(const eckit::option::CmdArgs& args) override;

    void init(const CmdArgs& args) override;

    void usage(const std::string& tool) const override;

private:  // members

    std::vector<PathName> paths_;
    std::string verb_     = "retrieve";
    std::string database_ = "";
    std::string source_   = "";
    std::string target_   = "";
    bool one_             = false;
    bool compact_         = false;
    bool json_            = false;
};

//----------------------------------------------------------------------------------------------------------------------

void GribToRequestTool::init(const CmdArgs& args) {
    args.get("one", one_);
    args.get("compact", compact_);
    args.get("verb", verb_);
    args.get("database", database_);
    args.get("source", source_);
    args.get("target", target_);
    args.get("json", json_);

    if (json_) {
        porcelain_ = true;
    }

    if (one_ and compact_) {
        Log::error() << "Options --one and --compact are mutually exclusive" << std::endl;
        std::exit(1);
    }
}


void GribToRequestTool::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] [request1] [request2] ..." << std::endl << std::endl;

    Log::info() << "Examples:" << std::endl
                << "=========" << std::endl
                << std::endl
                << tool << " --one --verb=retrieve data.grib" << std::endl
                << tool << " --compact --verb=retrieve data.grib" << std::endl
                << std::endl;
}

namespace {
static void toJSON(const std::vector<MarsRequest>& requests) {
    JSON j(Log::info());
    for (const auto& r : requests) {
        r.json(j);
    }
    Log::info() << std::endl;
}

static void toStdOut(const std::vector<MarsRequest>& requests) {
    for (const auto& r : requests) {
        eckit::Log::info() << r << std::endl;
    }
}

static void addKeyValue(std::vector<MarsRequest>& requests, const std::string& key, const std::string& value) {
    std::transform(requests.begin(), requests.end(), requests.begin(), [key, value](MarsRequest& r) -> MarsRequest {
        r.setValue(key, value);
        return r;
    });
}
}  // namespace

void GribToRequestTool::execute(const eckit::option::CmdArgs& args) {
    PathName inFile(args(0));

    FileHandle dh(inFile);
    dh.openForRead();

    eckit::message::Reader reader(dh, false);
    eckit::message::Message msg;

    std::vector<MarsRequest> requests;

    while ((msg = reader.next())) {
        MarsRequest r(verb_);
        MarsRequestSetter setter(r);

        msg.getMetadata(setter);

        if (one_ and requests.size()) {
            requests.back().merge(r);
        }
        else {
            requests.push_back(r);
        }
    }

    if (compact_ && requests.size() > 1) {
        std::map<std::set<std::string>, std::vector<MarsRequest>> coherentRequests;

        // split the requests into groups of requests with the same set of metadata (but potentially different values)
        for (const auto& r : requests) {
            std::set<std::string> keys;
            for (const auto& p : r.parameters()) {
                keys.insert(p.name());
            }
            coherentRequests[keys].push_back(r);
        }
        requests.clear();

        // compact each group of requests into a single request (if possible)
        for (const auto& [keys, reqs] : coherentRequests) {
            if (reqs.size() == 1) {  // it is a single field - return its request as is
                requests.push_back(reqs.front());
                continue;
            }

            MarsRequest merged{reqs.front()};
            for (size_t i = 1; i < reqs.size(); ++i) {
                merged.merge(reqs[i]);
            }
            if (merged.count() ==
                reqs.size()) {  // the set of fields forms a full hypercube - return corresponding merged request
                requests.push_back(std::move(merged));
                continue;
            }

            // sparse hypercube - we have to compute a set of compact requests describing the input fields
            metkit::hypercube::HyperCube h{merged};
            for (const auto& r : reqs) {
                h.clear(r);
            }
            for (const auto& r : h.requests()) {
                requests.push_back(r);
            }
        }
    }

    if (not database_.empty()) {
        addKeyValue(requests, "database", database_);
    }
    if (StringTools::lower(verb_) == "archive") {
        addKeyValue(requests, "source", inFile);
    }
    if (not source_.empty()) {
        addKeyValue(requests, "source", source_);
    }
    if (not target_.empty()) {
        addKeyValue(requests, "target", target_);
    }
    if (json_) {
        toJSON(requests);
    }
    else {
        toStdOut(requests);
    }
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    GribToRequestTool tool(argc, argv);
    return tool.start();
}
