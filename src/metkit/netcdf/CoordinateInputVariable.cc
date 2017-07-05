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

#include "CoordinateInputVariable.h"

#include "CoordinateOutputVariable.h"
#include "ScalarCoordinateInputVariable.h"


CoordinateInputVariable::CoordinateInputVariable(Field &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions):
    InputVariable(owner, name, id, dimensions)
{
}

CoordinateInputVariable::~CoordinateInputVariable() {
}

Variable *CoordinateInputVariable::makeOutputVariable(Field &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const {
    return new CoordinateOutputVariable(owner, name, dimensions);
}

Variable *CoordinateInputVariable::makeCoordinateVariable() {
    return this;
}

Variable *CoordinateInputVariable::makeScalarCoordinateVariable() {
    Variable *v = new ScalarCoordinateInputVariable(owner_, name_, id_, dimensions_);
    v->copyAttributes(*this);
    v->setMatrix(matrix());
    return v;
}

void CoordinateInputVariable::print(std::ostream &out) const {
    out << "CoordinateInputVariable[name=" << name_ << "]";
}
