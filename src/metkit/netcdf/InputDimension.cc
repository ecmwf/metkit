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

#include "metkit/netcdf/InputDimension.h"

#include "metkit/netcdf/Field.h"
#include "metkit/netcdf/OutputDimension.h"

#include <ostream>

namespace metkit{
namespace netcdf{

InputDimension::InputDimension(Field &owner, const std::string &name, int id, size_t len):
    Dimension(owner, name, len),
    id_(id)
{
}

InputDimension::~InputDimension()
{

}

void InputDimension::clone(Field &owner) const {
    owner.add(new OutputDimension(owner, name_, len_));
}

void InputDimension::print(std::ostream &out) const {
    out << "InputDimension[name=" << name_ << "]";
}

int InputDimension::id() const {
    return id_;
}

}
}
