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

#include "metkit/netcdf/InputField.h"

#include "metkit/netcdf/Attribute.h"
#include "metkit/netcdf/Dimension.h"
#include "metkit/netcdf/Exceptions.h"
#include "metkit/netcdf/DataInputVariable.h"

#include <netcdf.h>

namespace metkit {
namespace netcdf {

InputField::InputField(const DataInputVariable &owner):
    Field(owner),
    owner_(owner)
{
}

InputField::~InputField() {
}

void InputField::print(std::ostream &out) const {
    out << "InputField[owner=" << owner_ << "]";
}

}
}
