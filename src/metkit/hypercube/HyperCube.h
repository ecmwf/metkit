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


#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <vector>

#include "eckit/utils/HyperCube.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/Dictionary.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit::hypercube {

class Axis;

class AxisOrder {
public:  // methods

    static AxisOrder& instance();

    const std::vector<std::string>& axes() { return axes_; }
    size_t index(const std::string& axis) const;

private:  // methods

    AxisOrder();

    eckit::PathName axisYamlFile() { return "~metkit/share/metkit/axis.yaml"; }

private:  // members

    std::vector<std::string> axes_;
    std::map<std::string, size_t> axisIndex_;
};

class HyperCube {
public:

    HyperCube(const metkit::mars::MarsValidatedRequest&);
    ~HyperCube();

    bool contains(const metkit::mars::MarsValidatedRequest&) const;
    bool clear(const metkit::mars::MarsValidatedRequest&);

    size_t count() const;
    size_t countVacant() const;
    size_t size() const { return cube_.count(); }

    size_t fieldOrdinal(const metkit::mars::MarsValidatedRequest&, bool noholes = true) const;
    std::vector<metkit::mars::MarsValidatedRequest> vacantRequests() const { return aggregatedRequests(true); }
    std::vector<metkit::mars::MarsValidatedRequest> requests() const { return aggregatedRequests(false); }

protected:

    std::vector<metkit::mars::MarsValidatedRequest> aggregatedRequests(bool remaining) const;
    int indexOf(const metkit::mars::MarsValidatedRequest&) const;
    bool clear(int index);
    metkit::mars::MarsValidatedRequest requestOf(size_t index) const;

    // Given a set of indices, build the *minimal* collection of Mars requests that cover them.
    // Each entry in the result vector is: { merged_request, number_of_points_covered_by_that_request }
    /// @note: This does not take into account whether the point is "set" or not
    std::vector<std::pair<metkit::mars::MarsValidatedRequest, size_t>> request(const std::set<size_t>& idxs) const;

private:

    mars::Verb verb_;
    std::vector<Axis*> axes_;
    std::map<mars::Keyword, Axis*> axesByName_;
    std::vector<bool> set_;
    eckit::HyperCube cube_;
    size_t count_;


    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const HyperCube& p) {
        p.print(s);
        return s;
    }
};

}  // namespace metkit::hypercube
