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
#include "eckit/parser/YAMLParser.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/Type.h"

namespace metkit::hypercube {

AxisOrder::AxisOrder() {
    eckit::Value axis            = eckit::YAMLParser::decodeFile(axisYamlFile());
    const eckit::Value axesNames = axis["axes"];

    for (size_t i = 0; i < axesNames.size(); ++i) {
        axisIndex_[axesNames[i]] = i + 1;
        axes_.push_back(axesNames[i]);
    }
}

size_t AxisOrder::index(const std::string& axis) const {
    auto it = axisIndex_.find(axis);
    if (it == axisIndex_.end()) {
        return 0;
    }
    return it->second;
}

AxisOrder& AxisOrder::instance() {
    static AxisOrder instance;
    return instance;
}


class Axis {
public:

    Axis(const std::string& name, const std::vector<std::string>& values) : name_(name), values_(values) {}

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

    friend std::ostream& operator<<(std::ostream& s, const Axis& a) {
        s << "Axis[" << a.name_ << "]:";
        for (size_t i = 0; i < a.values_.size(); ++i) {
            s << " " << a.values_[i];
        }
        return s;
    }

private:

    std::string name_;
    std::vector<std::string> values_;
};

HyperCube::HyperCube(const metkit::mars::MarsRequest& request) :
    verb_(request.verb()), cube_(std::vector<eckit::Ordinal>()) {

    std::vector<eckit::Ordinal> dimensions;

    for (auto& name : AxisOrder::instance().axes()) {
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
    ADJACENT,
    DISJOINT
};

requestRelation getRelation(const metkit::mars::MarsRequest& base, const size_t& baseSize,
                            const metkit::mars::MarsRequest& additional, const size_t additionalSize) {

    metkit::mars::MarsRequest tmp(base);
    tmp.merge(additional);  // creates the bounding box request

    /// @todo: building a hypercube *JUST* to get the size? Seems like we can do better
    size_t sizeAfter = tmp.count();

    if (sizeAfter == baseSize)
        return requestRelation::EMBEDDED;

    if (baseSize + additionalSize == sizeAfter)
        return requestRelation::ADJACENT;

    return requestRelation::DISJOINT;
}

// Returns true only if the last request was merged into an adjacent
bool mergeLast(std::vector<std::pair<metkit::mars::MarsRequest, size_t>>& requests) {
    size_t last = requests.size() - 1;

    size_t candidateIdx  = std::numeric_limits<size_t>::max();
    size_t candidateSize = 0;

    // check if the new request is embedded or adjacent to existing requests
    for (size_t j = 0; j < requests.size() - 1; j++) {
        requestRelation relation =
            getRelation(requests[j].first, requests[j].second, requests[last].first, requests[last].second);
        if (relation == requestRelation::EMBEDDED) {
            requests.pop_back();
            return false;
        }
        if (relation == requestRelation::ADJACENT) {
            // We can merge the two requests
            // Though we will only merge with the largest adjacent request, so dont merge yet.
            size_t combinedSize = requests[j].second + requests[last].second;
            if (candidateSize < combinedSize) {
                candidateIdx  = j;
                candidateSize = combinedSize;
            }
        }
    }

    // Merge with the largest adjacent request if one was found
    if (candidateIdx != std::numeric_limits<size_t>::max()) {
        requests[candidateIdx].first.merge(requests[last].first);
        requests[candidateIdx].second += requests[last].second;
        requests.pop_back();
        return true;
    }

    return false;
}

std::vector<std::pair<mars::MarsRequest, std::size_t>> HyperCube::request(const std::set<std::size_t>& idxs) const {

    using IndexSet = std::set<std::size_t>;

    ASSERT(idxs.size() > 0);

    if (idxs.size() <= 1) {
        return {{requestOf(*idxs.begin()), 1}};
    }

    // -- helper lambdas --------------------------------------------

    // Partition the cube into a set of slices along the given axis.
    const auto sliceAlongAxis = [&](const IndexSet& set, std::size_t axis) {
        std::map<eckit::Ordinal, IndexSet> slices;
        std::vector<eckit::Ordinal> coords(axes_.size());

        for (std::size_t idx : set) {
            cube_.coordinates(idx, coords);
            slices[coords[axis]].insert(idx);
        }

        return slices;
    };

    // Pick the axis which can be partitioned into the smallest number of slices (>1).
    const auto pickBestAxis = [&](const IndexSet& set) -> std::size_t {
        std::size_t bestAxis    = 0;
        std::size_t bestNSlices = std::numeric_limits<std::size_t>::max();

        for (std::size_t axis = 0; axis < axes_.size(); ++axis) {
            const std::size_t nSlices = sliceAlongAxis(set, axis).size();
            if (nSlices > 1 && nSlices < bestNSlices) {
                bestAxis    = axis;
                bestNSlices = nSlices;
            }
        }

        ASSERT(bestNSlices != std::numeric_limits<std::size_t>::max());
        return bestAxis;
    };

    // ---------------------------------------------------------------

    const std::size_t axis = pickBestAxis(idxs);
    const auto slices      = sliceAlongAxis(idxs, axis);

    std::vector<std::pair<mars::MarsRequest, std::size_t>> result;

    // Process each slice recursively, appending and merging on the fly.
    for (const auto& [coord, subIdxs] : slices) {
        auto subRequests = request(subIdxs);  // recursion

        result.insert(result.end(), subRequests.begin(), subRequests.end());
        while (mergeLast(result)) {}
    }

    return result;
}

std::vector<metkit::mars::MarsRequest> HyperCube::aggregatedRequests(bool remaining) const {

    if (countVacant() == (remaining ? 0 : size()))
        return std::vector<metkit::mars::MarsRequest>{};

    std::set<size_t> idxs;
    for (size_t i = 0; i < set_.size(); ++i) {
        if (set_[i] == remaining)
            idxs.emplace(i);
    }

    std::vector<std::pair<metkit::mars::MarsRequest, size_t>> requests = request(idxs);

    std::vector<metkit::mars::MarsRequest> out;
    for (auto req : requests)
        out.push_back(req.first);
    return out;
}

metkit::mars::MarsRequest HyperCube::requestOf(size_t index) const {
    metkit::mars::MarsRequest request(verb_);
    std::vector<eckit::Ordinal> coords(axes_.size());

    cube_.coordinates(index, coords);
    for (size_t i = 0; i < axes_.size(); i++) {
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

void HyperCube::print(std::ostream& s) const {
    NOTIMP;  ///@todo
}

}  // namespace metkit::hypercube
