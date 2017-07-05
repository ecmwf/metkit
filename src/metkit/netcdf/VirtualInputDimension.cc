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

#include "VirtualInputDimension.h"

#include "Field.h"
#include "VirtualOutputDimension.h"

#include <ostream>

namespace metkit{
namespace netcdf{
VirtualInputDimension::VirtualInputDimension(Field &owner, const std::string &name):
    Dimension(owner, name, 1)
{
}

VirtualInputDimension::~VirtualInputDimension()
{
}

void VirtualInputDimension::clone(Field &owner) const {
    owner.add(new VirtualOutputDimension(owner, name_));
}

void VirtualInputDimension::print(std::ostream &out) const {
    out << "VirtualInputDimension[name=" << name_ << "]";
}

int VirtualInputDimension::id() const {
    return -1;
}
}
}
