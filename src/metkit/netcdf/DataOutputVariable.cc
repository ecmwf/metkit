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

#include "metkit/netcdf/DataOutputVariable.h"

#include "metkit/netcdf/Attribute.h"
#include "metkit/netcdf/MergeDataStep.h"
#include "metkit/netcdf/MergePlan.h"

namespace metkit {
namespace netcdf {

DataOutputVariable::DataOutputVariable(Field &owner, const std::string &name, const std::vector<Dimension *> &dimensions):
    OutputVariable(owner, name, dimensions)
{
}

DataOutputVariable::~DataOutputVariable() {

}

void DataOutputVariable::print(std::ostream &out) const {
    out << "DataOutputVariable[name=" << name_ << ",nc=" << ncname() << ",cube=" << cube() << "]";
}

void DataOutputVariable::merge(const Variable &other, MergePlan &plan)
{
    Variable::merge(other, plan);
    plan.add(new MergeDataStep(*this, other));
}

const std::string &DataOutputVariable::ncname() const {
    std::map<std::string, Attribute *>::const_iterator j = attributes_.find("standard_name");
    if (j != attributes_.end()) {
        ncname_ = (*j).second->asString();
        return ncname_;
    }
    return name();
}

}
}
