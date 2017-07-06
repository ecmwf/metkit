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

#include "metkit/netcdf/DummyInputVariable.h"

#include "metkit/netcdf/Attribute.h"
#include "metkit/netcdf/DummyMatrix.h"
#include "metkit/netcdf/Exceptions.h"

#include <iostream>

namespace metkit {
namespace netcdf {

DummyInputVariable::DummyInputVariable(Dataset &owner, const Variable &parent):
    Variable(owner, parent.name(), parent.dimensions()),
    parent_(parent)
{
    std::cout << "DummmyInputVariable " << parent_ << std::endl;
    setMatrix(new DummyMatrix(parent));

    for (std::map<std::string, Attribute *>::const_iterator j = parent.attributes().begin(); j != parent.attributes().end(); ++j)
    {
        (*j).second->clone(*this);
    }

}

DummyInputVariable::~DummyInputVariable()
{
}

const std::string &DummyInputVariable::ncname() const {
    return parent_.ncname();
}


int DummyInputVariable::varid() const
{
    NOTIMP;
    return 0;
}

void DummyInputVariable::print(std::ostream &out) const {
    out << "DummyInputVariable[name=" << name_ << "]";
}


Variable *DummyInputVariable::makeOutputVariable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const {
    NOTIMP;
    return 0;
}

bool DummyInputVariable::dummy() const {
    return true;
}

bool DummyInputVariable::sameAsDummy(const Variable &other) const {
    return &parent_ == &other;
}

}
}
