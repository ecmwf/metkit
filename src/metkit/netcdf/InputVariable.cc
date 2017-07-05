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

#include "InputVariable.h"

#include "Attribute.h"
#include "Dimension.h"
#include "Exceptions.h"
#include "Field.h"
namespace metkit{
namespace netcdf{

InputVariable::InputVariable(Field &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions):
    Variable(owner, name, dimensions),
    id_(id)
{
}

InputVariable::~InputVariable()
{
}

int InputVariable::varid() const {
    ASSERT(id_ >= 0);
    return id_;
}

Variable *InputVariable::clone(Field &owner) const {

    std::vector<Dimension *> dimensions;
    for (std::vector<Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j) {
        dimensions.push_back(owner.findDimension((*j)->name()));
    }

    Variable *v = makeOutputVariable(owner, name_, dimensions);
    v->setMatrix(matrix_);

    for (std::map<std::string, Attribute *>::const_iterator j = attributes_.begin(); j != attributes_.end(); ++j)
    {
        (*j).second->clone(*v);
    }

    owner.add(v);

    return v;
}

void InputVariable::print(std::ostream &out) const {
    out << "InputVariable[name=" << name_ << "]";
}

}
}
