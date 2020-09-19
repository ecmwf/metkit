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
//        std::cout << "Axis::valueOf [axis=" << name() << ",index=" << index << " of " << values_.size() << ",value=" << values_[index] << "]" << std::endl;
        return values_[index];
    }

private:
    std::string name_;
    std::vector<std::string> values_;
    std::vector<bool> set_;
    metkit::mars::Type& type_;
};


HyperCube::HyperCube(const metkit::mars::MarsRequest& request) : cube_(std::vector<eckit::Ordinal>()) {

    // std::set<std::string> seen;
    // metkit::mars::MarsRequest data = request.extract("data");

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

bool HyperCube::clear(const metkit::mars::MarsRequest& r) {
    int idx = indexOf(r);
    if (idx < 0)
        return false;
    if (!set_[idx])
        return false;
    set_[idx] = false;
    count_--;
    return true;
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

metkit::mars::MarsRequest HyperCube::request() const {
    metkit::mars::MarsRequest request("retrieve");
    std::vector<eckit::Ordinal> coords(axes_.size());

//    cube_.coordinates(index, coords);
    for (size_t i=0; i<axes_.size(); i++) {
        request.setValue(axes_[i]->name(), axes_[i]->valueOf(coords[i]));
    }
    return request;
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
