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
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "eckit/io/Buffer.h"
#include "eckit/io/Offset.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"

#include "metkit/metkit_config.h"

#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/tool/MetkitTool.h"

using namespace eckit;
using namespace eckit::option;
using namespace metkit;
using namespace metkit::mars;

#ifdef metkit_HAVE_MARS2MARS
#include "metkit/mars2mars/api/Mars2Mars.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictaccess_mars_request.h"
#include "metkit/mars2mars/utils/dictionary_traits/dictionary_access_traits.h"

using namespace metkit::mars2mars;

namespace {
class ConvertedRequests : public FlattenCallback {
public:

    ConvertedRequests(std::vector<MarsRequest>& flattenedRequests) : flattenedRequests_(flattenedRequests) {}

    void operator()(const MarsRequest& req) override { flattenedRequests_.push_back(converter_.convert(req).mars); }

    std::vector<MarsRequest>& flattenedRequests_;
    Mars2Mars converter_;
};

}  // namespace

#endif

//----------------------------------------------------------------------------------------------------------------------

class ParseRequest : public MetkitTool {
public:

    ParseRequest(int argc, char** argv) : MetkitTool(argc, argv) {
        options_.push_back(new SimpleOption<bool>("json", "Format request in json, default = false"));
        options_.push_back(new SimpleOption<bool>("compact", "Compact output, default = false"));
#ifdef metkit_HAVE_MARS2MARS
        options_.push_back(
            new SimpleOption<bool>("convert-to-grib2", "Convert request to the full Grib2 metadata, default = false"));
#endif
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
    bool grib2_   = false;
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
    args.get("convert-to-grib2", grib2_);
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
#ifdef metkit_HAVE_MARS2MARS
                << tool << " --convert-to-grib2 mars1.req mars2.req" << std::endl
#endif
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

#ifdef metkit_HAVE_MARS2MARS
    if (grib2_) {
        std::vector<MarsRequest> out;

        for (auto& r : v) {
            std::vector<MarsRequest> converted;
            ConvertedRequests cb(converted);
            expand.flatten(r, cb);

            if (converted.size() > 1) {
                std::map<std::set<std::string>, std::vector<MarsRequest>> coherentRequests;

                // split the requests into groups of requests with the same set of metadata (but potentially different
                // values)
                for (const auto& r : converted) {
                    std::set<std::string> keys;
                    for (const auto& p : r.parameters()) {
                        keys.insert(p.name());
                    }
                    coherentRequests[keys].push_back(r);
                }
                converted.clear();

                // compact each group of requests into a single request (if possible)
                for (const auto& [keys, reqs] : coherentRequests) {
                    if (reqs.size() == 1) {  // it is a single field - return its request as is
                        converted.push_back(reqs.front());
                        continue;
                    }

                    MarsRequest merged{reqs.front()};
                    for (size_t i = 1; i < reqs.size(); ++i) {
                        merged.merge(reqs[i]);
                    }
                    if (merged.count() == reqs.size()) {  // the set of fields forms a full hypercube - return
                                                          // corresponding merged request
                        converted.push_back(std::move(merged));
                        continue;
                    }

                    // sparse hypercube - we have to compute a set of compact requests describing the input fields
                    metkit::hypercube::HyperCube h{merged};
                    for (const auto& r : reqs) {
                        h.clear(r);
                    }
                    for (const auto& r : h.requests()) {
                        converted.push_back(r);
                    }
                }
            }
            out.insert(out.end(), converted.begin(), converted.end());
        }
        std::swap(v, out);
    }
#endif  // HAVE_MARS2MARS


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
}

//----------------------------------------------------------------------------------------------------------------------

int main(int argc, char** argv) {
    ParseRequest tool(argc, argv);
    return tool.start();
}
