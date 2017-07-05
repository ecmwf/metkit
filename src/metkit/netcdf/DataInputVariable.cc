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

#include "metkit/netcdf/DataInputVariable.h"
#include "metkit/netcdf/Attribute.h"
#include "metkit/netcdf/DataOutputVariable.h"

namespace metkit {
namespace netcdf {

DataInputVariable::DataInputVariable(Field &owner,
                                     const std::string &name,
                                     int id,
                                     const std::vector<Dimension *> &dimensions):
    InputVariable(owner, name, id, dimensions)
{
}

DataInputVariable::~DataInputVariable() {
}

Variable *DataInputVariable::makeOutputVariable(Field &owner,
        const std::string &name,
        const std::vector<Dimension *> &dimensions) const {
    return new DataOutputVariable(owner, name, dimensions);
}

void DataInputVariable::print(std::ostream &out) const {
    out << "DataInputVariable[name=" << name_ << ",nc=" << ncname() << ",cube=" << cube() << "]";
}

const std::string &DataInputVariable::ncname() const {
    std::map<std::string, Attribute *>::const_iterator j = attributes_.find("standard_name");
    if (j != attributes_.end()) {
        ncname_ = (*j).second->asString();
        return ncname_;
    }
    return name();
}

}
}
