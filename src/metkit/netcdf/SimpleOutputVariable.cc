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

#include "metkit/netcdf/SimpleOutputVariable.h"

namespace metkit{
namespace netcdf{

SimpleOutputVariable::SimpleOutputVariable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions):
    OutputVariable(owner, name, dimensions)
{
}

SimpleOutputVariable::~SimpleOutputVariable() {
}

void SimpleOutputVariable::print(std::ostream &out) const {
    out << "SimpleOutputVariable[name=" << name_ << "]";
}

}
}
