/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/hypercube/HyperCube.h"

#include <algorithm>

#include "eckit/exception/Exceptions.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"

namespace metkit {
namespace hypercube {


static const std::vector<std::string> axis_order = {

    "class",      "type",      "stream",     "levtype",

    "origin",     "product",   "section",    "method",   "system",

    "date",       "refdate",   "hdate",      "time",     "anoffset",

    "reference",  "step",      "fcmonth",    "fcperiod",

    "leadtime",   "opttime",

    "expver",     "domain",

    "diagnostic", "iteration",

    "quantile",   "number",

    "levelist",   "latitude",  "longitude",  "range",

    "param",

    "ident",      "obstype",   "instrument",

    "frequency",  "direction",

    "channel",
};


static metkit::mars::Type& type(const std::string& name) {
    static metkit::mars::MarsLanguage language("retrieve");
    return *language.type(name);
}

class Axis {
public:
    Axis(const std::string& name, const std::vector<std::string>& values) :
        name_(name), values_(values), type_(type(name)) {}

    size_t size() const { return values_.size(); }

    const std::string& name() const { return name_; }

    int indexOf(const std::string& v) const {
        auto j = std::find(values_.begin(), values_.end(), v);
        if (j == values_.end()) {
            return -1;
        }
        return j - values_.begin();
    }

    const std::string& valueOf(size_t index) const {
        if (values_.size() <= index) {
            std::ostringstream oss;
            oss << "Axis::valueOf no value for [axis=" << name() << ",index=" << index << "]";
            throw eckit::UserError(oss.str());
        }
        return values_[index];
    }

private:
    std::string name_;
    std::vector<std::string> values_;
    std::vector<bool> set_;
    metkit::mars::Type& type_;
};


HyperCube::HyperCube(const metkit::mars::MarsRequest& request) : cube_(std::vector<eckit::Ordinal>()) {

    std::vector<eckit::Ordinal> dimensions;

    for (auto& name : axis_order) {
        std::vector<std::string> values = request.values(name, true);

        if (!values.empty()) {
            Axis* a = new Axis(name, values);
            axes_.push_back(a);
            axesByName_[name] = a;
            dimensions.push_back(values.size());
        }
    }

    cube_  = eckit::HyperCube(dimensions);
    count_ = cube_.count();
    set_   = std::vector<bool>(count_, true);
}

HyperCube::~HyperCube() {
    for (auto& a : axes_) {
        delete a;
    }
}

bool HyperCube::contains(const metkit::mars::MarsRequest& r) const {
    int idx = indexOf(r);
    return (idx >= 0) and set_[idx];
}

bool HyperCube::clear(int idx) {
    if (idx < 0)
        return false;
    if (!set_[idx])
        return false;
    set_[idx] = false;
    count_--;
    return true;
}
bool HyperCube::clear(const metkit::mars::MarsRequest& r) {
    int idx = indexOf(r);
    return clear(idx);
}

int HyperCube::indexOf(const metkit::mars::MarsRequest& r) const {

    std::vector<eckit::Ordinal> coords;

    for (auto& a : axes_) {
        const std::vector<std::string>& values = r.values(a->name(), true);
        if (values.size() == 0) {
            std::ostringstream oss;
            oss << "HyperCube::indexOf no value for [" << a->name() << "] in request " << r;
            throw eckit::UserError(oss.str());
        }

        if (values.size() > 1) {
            std::ostringstream oss;
            oss << "HyperCube::indexOf too many values for [" << a->name() << "] in request " << r;
            throw eckit::UserError(oss.str());
        }

        int n = a->indexOf(values[0]);
        if (n < 0) {
            return -1;
        }

        coords.push_back(n);
    }

    return cube_.index(coords);
}

enum requestRelation {
    EMBEDDED,
    ADIACENT,
    DISJOINT
};

requestRelation getRelation(const metkit::mars::MarsRequest& base, const size_t& baseSize, const metkit::mars::MarsRequest& additional, const size_t additionalSize) {

    metkit::mars::MarsRequest tmp(base);
    tmp.merge(additional);
    size_t sizeAfter = HyperCube(tmp).size();

    if (sizeAfter == baseSize)
        return requestRelation::EMBEDDED;

    if (baseSize + additionalSize == sizeAfter)
        return requestRelation::ADIACENT;

    return requestRelation::DISJOINT;
}

bool mergeLast(std::vector<std::pair<metkit::mars::MarsRequest, size_t>>& requests) {
    size_t last = requests.size()-1;

    int candidateIdx = -1;
    int candidateSize = -1;
    requestRelation relation = requestRelation::DISJOINT;

    // check id the new request is embedded or adiacent to existing requests
    for (size_t j = 0; j < requests.size()-1; j++) {
        relation = std::min(relation, getRelation(requests[j].first, requests[j].second, requests[last].first, requests[last].second));
        if (relation == requestRelation::EMBEDDED) {
            requests.pop_back();
            return false;
        }
        if (relation == requestRelation::ADIACENT) {
            // try to merge in the largest request
            if (candidateSize == -1 || candidateSize < requests[j].second + requests[last].second) {
                candidateIdx = j;
                candidateSize = requests[j].second + requests[last].second;
            }
        }
    }
    if (relation == requestRelation::EMBEDDED) {
        requests.pop_back();
        return false;
    }
    if (relation == requestRelation::ADIACENT && candidateIdx != -1) {
        requests[candidateIdx].first.merge(requests[last].first);
        requests[candidateIdx].second += requests[last].second;
        requests.pop_back();
        return true;
    }
    return false;
}


std::vector<std::pair<metkit::mars::MarsRequest, size_t>> HyperCube::request(std::set<size_t> idxs) const {
    size_t min = 1;
    int minAxis = -1;
    std::vector<std::map<eckit::Ordinal, std::set<size_t>>> axes(axes_.size());

    if (idxs.size() > 1) {
        std::vector<eckit::Ordinal> coords(axes_.size());
        for (size_t idx: idxs) {
            cube_.coordinates(idx, coords);
            for(size_t j=0; j<axes_.size(); j++) {
                axes[j][coords[j]].emplace(idx);
                if (1 < axes[j].size() && (axes[j].size() < min || min == 1)) {
                    min = axes[j].size();
                    minAxis = j;
                }
            }
        }
    }
    // all requests are identical, returning just the first
    if (min == 1)
        return std::vector<std::pair<metkit::mars::MarsRequest, size_t>> {std::make_pair<metkit::mars::MarsRequest, size_t>(requestOf(*idxs.begin()), 1)};

    auto it = axes[minAxis].begin();
    std::vector<std::pair<metkit::mars::MarsRequest, size_t>> requests = request(it->second);
    for(; it != axes[minAxis].end(); it++) {
        std::vector<std::pair<metkit::mars::MarsRequest, size_t>> toAdd = request(it->second);
        for(auto r: toAdd) {
            requests.push_back(r);
            while (mergeLast(requests));
        }
    }
    return requests;
}

std::vector<metkit::mars::MarsRequest> HyperCube::vacantRequests() const {

    if (countVacant() == 0)
        return std::vector<metkit::mars::MarsRequest>{};

    std::set<size_t> idxs;
    for(size_t i = 0; i < set_.size(); ++i) {
        if (set_[i])
            idxs.emplace(i);
    }

    std::vector<std::pair<metkit::mars::MarsRequest, size_t>> requests = request(idxs);

    std::vector<metkit::mars::MarsRequest> out;
    for (auto req: requests)
        out.push_back(req.first);
    return out;
}

metkit::mars::MarsRequest HyperCube::requestOf(size_t index) const {
    metkit::mars::MarsRequest request("retrieve");
    std::vector<eckit::Ordinal> coords(axes_.size());

    cube_.coordinates(index, coords);
    for (size_t i=0; i<axes_.size(); i++) {
        request.setValue(axes_[i]->name(), axes_[i]->valueOf(coords[i]));
    }
    return request;
}

size_t HyperCube::count() const {
    return count_;
}
size_t HyperCube::countVacant() const {
    return count_;
}

size_t HyperCube::fieldOrdinal(const metkit::mars::MarsRequest& r, bool noholes) const {
    int idx = indexOf(r);
    ASSERT(idx >= 0);
    if (noholes) {
        size_t p = 0;
        for (size_t i = 0; i < idx; ++i) {
            if (set_[i]) {
                p++;
            }
        }
        return p;
    }
    return idx;
}

}  // namespace hypercube
}  // namespace metkit
