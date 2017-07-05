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

#include "CoordinateOutputVariable.h"

#include "Matrix.h"
#include "Type.h"

CoordinateOutputVariable::CoordinateOutputVariable(Field &owner, const std::string &name, const std::vector<Dimension *> &dimensions):
    OutputVariable(owner, name, dimensions)
{
}

CoordinateOutputVariable::~CoordinateOutputVariable() {

}

void CoordinateOutputVariable::print(std::ostream &out) const {
    out << "CoordinateOutputVariable[name=" << name_ << "]";
}


void CoordinateOutputVariable::merge(const Variable &other, MergePlan &plan)
{

    Variable::merge(other, plan);

    Type &type1 = matrix()->type();
    Type &type2 = other.matrix()->type();

    Type &common = Type::lookup(type1, type2);
    if (common.coordinateOutputVariableMerge(*this, other, plan)) {
        // Merge done
    }
    else {
        if (type1 != type2) {
            // Type change
            matrix()->type(common);
        }
        else {
            // No change
        }

    }
}
