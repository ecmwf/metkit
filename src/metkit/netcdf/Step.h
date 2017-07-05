/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#ifndef metkit_netcdf_Step
#define metkit_netcdf_Step

#include <iosfwd>

namespace metkit {
namespace netcdf {

class MergePlan;


class Step {
public:
    Step();
    virtual ~Step();

    virtual int rank() const = 0;
    virtual void execute(MergePlan &plan) = 0;
    virtual bool merge(Step *other);

private:
    // -- Methods

    virtual void print(std::ostream &s) const = 0;

    // - Friend
    friend std::ostream &operator<<(std::ostream &s, const Step &v)
    {
        v.print(s);
        return s;
    }
};

struct CompareSteps {
    bool operator()(const Step *a, const Step *b) const {
        return a->rank() > b->rank();
    }
};

}
}
#endif
