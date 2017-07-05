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

#include "ScalarCoordinateInputVariable.h"

#include "Field.h"
#include "ScalarCoordinateOutputVariable.h"
#include "VirtualInputDimension.h"

namespace metkit{
namespace netcdf{
ScalarCoordinateInputVariable::ScalarCoordinateInputVariable(Field &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions):
    InputVariable(owner, name, id, dimensions)
{
}

ScalarCoordinateInputVariable::~ScalarCoordinateInputVariable() {
}

Variable *ScalarCoordinateInputVariable::makeOutputVariable(Field &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const {
    return new ScalarCoordinateOutputVariable(owner, name, dimensions);
}

Variable *ScalarCoordinateInputVariable::makeCoordinateVariable() {
    return this;
}

void ScalarCoordinateInputVariable::print(std::ostream &out) const {
    out << "ScalarCoordinateInputVariable[name=" << name_ << "]";
}

Dimension *ScalarCoordinateInputVariable::getVirtualDimension() {
    if (dimensions_.size() == 0) {
        Dimension *dim = new VirtualInputDimension(owner_, name_);
        owner_.add(dim);
        dimensions_.push_back(dim);
        resetCube();
    }
    return dimensions_[0];
}
}
}
