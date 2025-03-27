/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Aug 2017


#ifndef metkit_HyperCube_H
#define metkit_HyperCube_H

#include <iosfwd>
#include <map>
#include <memory>
#include <vector>

#include "eckit/utils/HyperCube.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit {
namespace hypercube {

class Axis;

class AxisOrder {
public:  // methods

    static AxisOrder& instance();

    const std::vector<std::string>& axes() { return axes_; }

private:  // methods

    AxisOrder();

    eckit::PathName axisYamlFile() { return "~metkit/share/metkit/axis.yaml"; }

private:  // members

    std::vector<std::string> axes_;
};

class HyperCube {
public:

    HyperCube(const metkit::mars::MarsRequest&);
    ~HyperCube();

    bool contains(const metkit::mars::MarsRequest&) const;
    bool clear(const metkit::mars::MarsRequest&);

    size_t count() const;
    size_t countVacant() const;
    size_t size() const { return cube_.count(); }

    size_t fieldOrdinal(const metkit::mars::MarsRequest&, bool noholes = true) const;
    std::vector<metkit::mars::MarsRequest> vacantRequests() const { return aggregatedRequests(true); }
    std::vector<metkit::mars::MarsRequest> requests() const { return aggregatedRequests(false); }

protected:

    std::vector<metkit::mars::MarsRequest> aggregatedRequests(bool remaining) const;
    int indexOf(const metkit::mars::MarsRequest&) const;
    bool clear(int index);
    metkit::mars::MarsRequest requestOf(size_t index) const;
    std::vector<std::pair<metkit::mars::MarsRequest, size_t>> request(std::set<size_t> idxs) const;

private:

    std::string verb_;
    std::vector<Axis*> axes_;
    std::map<std::string, Axis*> axesByName_;
    std::vector<bool> set_;
    eckit::HyperCube cube_;
    size_t count_;


    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const HyperCube& p) {
        p.print(s);
        return s;
    }
};

}  // namespace hypercube
}  // namespace metkit


#endif
