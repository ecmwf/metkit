/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "ParseRequest.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/utils/StringTools.h"

#include "metkit/metkit_config.h"

#include "metkit/hypercube/HyperCube.h"
#include "metkit/mars/MarsExpansion.h"
#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsParser.h"
#include "metkit/mars/MarsRequest.h"

using namespace eckit;
using namespace metkit::mars;

#ifdef metkit_HAVE_MARS2MARS
#include "metkit/mars2mars/api/Mars2Mars.h"

using namespace metkit::mars2mars;

namespace {

/// Keys read or rewritten by the mars2mars rules (see mars2mars/mappings/rules/*.h). Only these keys are
/// flattened before conversion; every other key keeps its full list of values. Must be kept in sync with
/// the rules: a rule reading a key not listed here would see a list instead of a single value.
const std::vector<std::string>& ruleKeys() {
    static const std::vector<std::string> keys{"param",   "class",    "stream", "type",
                                               "levtype", "levelist", "step",   "timespan"};
    return keys;
}

/// The rules only read levelist for surface-like levtypes (sfc2sol), so levelist is not flattened for
/// multi-level levtypes.
bool flattenLevelist(const MarsRequest& request) {
    static const std::set<std::string> multiLevel{"pl", "ml", "pt", "pv", "hl", "o3d"};
    return !(request.countValues("levtype") == 1 && multiLevel.count(request.values("levtype").front()) > 0);
}

void flattenRuleKeys(size_t i, MarsRequest& current, const std::function<void(const MarsRequest&)>& callback) {
    if (i == ruleKeys().size()) {
        callback(current);
        return;
    }

    const std::string& key = ruleKeys()[i];
    if (current.countValues(key) <= 1 || (key == "levelist" && !flattenLevelist(current))) {
        flattenRuleKeys(i + 1, current, callback);
        return;
    }

    const std::vector<std::string> values = current.values(key);
    for (const auto& v : values) {
        current.setValue(key, v);
        flattenRuleKeys(i + 1, current, callback);
    }
    current.values(key, values);
}

/// A converted (partial) request and the position of its first field in the input request.
struct ConvertedRequest {
    MarsRequest request;
    size_t order;
};

using OrderedRequests = std::vector<std::pair<MarsRequest, size_t>>;

/// Compact a group of converted requests sharing the same set of keys. Keys whose values are identical in
/// all requests of the group are copied from the first request, so keys that are not hypercube axes
/// (grid, area, target, ...) are preserved.
void compactGroup(const std::vector<const ConvertedRequest*>& members, OrderedRequests& out) {
    const MarsRequest& first = members.front()->request;

    size_t order = members.front()->order;
    for (const auto* m : members) {
        order = std::min(order, m->order);
    }

    std::vector<std::string> varying;
    for (const auto& name : first.params()) {
        for (const auto* m : members) {
            if (m->request.values(name) != first.values(name)) {
                varying.push_back(name);
                break;
            }
        }
    }

    if (varying.empty()) {
        out.emplace_back(first, order);
        return;
    }

    const auto emitIndividually = [&]() {
        for (const auto* m : members) {
            out.emplace_back(m->request, m->order);
        }
    };

    for (const auto* m : members) {
        for (const auto& name : varying) {
            if (m->request.countValues(name) != 1) {
                emitIndividually();
                return;
            }
        }
    }

    // Values of each varying key, in order of first appearance, and the set of distinct points
    std::vector<std::vector<std::string>> axes(varying.size());
    std::vector<std::unordered_set<std::string>> seen(varying.size());
    std::set<std::vector<std::string>> points;
    for (const auto* m : members) {
        std::vector<std::string> point;
        for (size_t j = 0; j < varying.size(); ++j) {
            const std::string& v = m->request.values(varying[j]).front();
            if (seen[j].insert(v).second) {
                axes[j].push_back(v);
            }
            point.push_back(v);
        }
        points.insert(std::move(point));
    }

    size_t denseSize = 1;
    for (const auto& a : axes) {
        denseSize *= a.size();
    }

    if (denseSize == points.size()) {
        MarsRequest merged{first};
        for (size_t j = 0; j < varying.size(); ++j) {
            merged.values(varying[j], axes[j]);
        }
        out.emplace_back(std::move(merged), order);
        return;
    }

    // Sparse: describe the points with a set of compact requests over the varying keys only
    const auto& axisNames = metkit::hypercube::AxisOrder::instance().axes();
    for (const auto& name : varying) {
        if (std::find(axisNames.begin(), axisNames.end(), name) == axisNames.end()) {
            emitIndividually();
            return;
        }
    }

    MarsRequest cubeRequest(first.verb());
    for (size_t j = 0; j < varying.size(); ++j) {
        cubeRequest.values(varying[j], axes[j]);
    }

    metkit::hypercube::HyperCube cube{cubeRequest};
    for (const auto* m : members) {
        cube.clear(m->request);
    }

    for (const auto& r : cube.requests()) {
        MarsRequest compact{first};
        std::vector<std::set<std::string>> values;
        for (const auto& name : varying) {
            compact.values(name, r.values(name));
            values.emplace_back(r.values(name).begin(), r.values(name).end());
        }

        size_t compactOrder = std::numeric_limits<size_t>::max();
        for (const auto* m : members) {
            bool contained = true;
            for (size_t j = 0; j < varying.size() && contained; ++j) {
                contained = values[j].count(m->request.values(varying[j]).front()) > 0;
            }
            if (contained) {
                compactOrder = std::min(compactOrder, m->order);
            }
        }

        out.emplace_back(std::move(compact), compactOrder);
    }
}

/// Convert a fully expanded request to GRIB2 (MTG2) MARS metadata and compact the result.
/// The output requests are sorted by the position of their first field in the input request.
std::vector<MarsRequest> convertRequest(const MarsRequest& request, Mars2Mars& converter) {
    std::vector<ConvertedRequest> converted;

    MarsRequest current{request};
    flattenRuleKeys(0, current,
                    [&](const MarsRequest& r) { converted.push_back({converter.convert(r).mars, converted.size()}); });

    // Group by set of keys, keeping the order in which the groups first appear
    std::vector<std::vector<const ConvertedRequest*>> groups;
    std::map<std::set<std::string>, size_t> groupIndex;
    for (const auto& c : converted) {
        const auto names = c.request.params();
        const std::set<std::string> keys(names.begin(), names.end());

        auto [it, inserted] = groupIndex.emplace(keys, groups.size());
        if (inserted) {
            groups.emplace_back();
        }
        groups[it->second].push_back(&c);
    }

    OrderedRequests compacted;
    for (const auto& g : groups) {
        compactGroup(g, compacted);
    }

    std::stable_sort(compacted.begin(), compacted.end(),
                     [](const auto& a, const auto& b) { return a.second < b.second; });

    std::vector<MarsRequest> result;
    result.reserve(compacted.size());
    for (auto& [r, order] : compacted) {
        result.push_back(std::move(r));
    }
    return result;
}

/// Split off the keys that do not describe fields (post-processing, sink and unknown keys). They play no
/// part in the conversion, so they are neither validated nor expanded, and are passed through verbatim.
/// Like MARS, a key is inherited by the following requests of the same verb until it is set to "off".
class PassThroughKeys {
public:

    MarsRequest split(MarsRequest& request) {
        const MarsLanguage& language = MarsLanguage::get(request.verb());
        MarsRequest& inherited       = inherited_.try_emplace(language.verb(), language.verb()).first->second;

        for (const auto& name : request.params()) {
            const std::string key = StringTools::lower(name);
            if (language.isData(key)) {
                continue;
            }

            const std::vector<std::string> values = request.values(name);
            request.unsetValues(name);

            if (values.size() == 1 && StringTools::lower(values.front()) == "off") {
                inherited.unsetValues(key);
            }
            else {
                inherited.values(key, values);
            }
        }

        return inherited;
    }

private:

    std::map<std::string, MarsRequest> inherited_;
};

}  // namespace

#endif

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
    if (porcelain_) {
        compact_ = true;
    }
}

void ParseRequest::usage(const std::string& tool) const {
    Log::info() << "Usage: " << tool << " [options] [request1] [request2] ..." << std::endl;
    if (grib2_) {
        Log::info() << "       Converts requests to full GRIB2 metadata." << std::endl;
    }

    Log::info() << std::endl
                << "Examples:" << std::endl
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

    std::vector<MarsRequest> v;

#ifdef metkit_HAVE_MARS2MARS
    if (grib2_) {
        PassThroughKeys passThrough;
        Mars2Mars converter;

        for (const auto& parsed : p) {
            MarsRequest fields{parsed};
            const MarsRequest extra = passThrough.split(fields);

            for (auto& r : convertRequest(expand.expand(fields), converter)) {
                for (const auto& param : extra.parameters()) {
                    r.values(param.name(), param.values());
                }
                v.push_back(std::move(r));
            }
        }
    }
    else
#endif  // HAVE_MARS2MARS
    {
        v = expand.expand(p);
    }

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
