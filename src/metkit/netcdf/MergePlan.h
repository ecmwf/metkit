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

#ifndef metkit_netcdf_MergePlan
#define metkit_netcdf_MergePlan

#include "metkit/netcdf/Step.h"

#include <queue>
#include <map>

namespace metkit{
namespace netcdf{

class Variable;
class Dimension;
class Field;

class MergePlan
{
public:
    MergePlan(Field &);
    ~MergePlan();

    void add(Step *);
    void execute();

    void link(const Variable &, const Variable &);
    const Variable &link(const Variable &);

    Field &field() const ;

private:

    Field &field_;

    MergePlan(const MergePlan &);
    MergePlan &operator=(const MergePlan &);

    // ----

    std::priority_queue<Step *, std::deque<Step *>, CompareSteps> queue_;
    std::vector<Step *> steps_;
    std::map<const Variable *, const Variable *> link_;

};

}
}
#endif
