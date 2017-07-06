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
#include "metkit/netcdf/Matrix.h"

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

std::string InputField::gridType() const {
    return "regular_ll";
}

long InputField::paramId() const {
    return 1;
}

double InputField::north() const {
    return 90;
}

double InputField::south() const {
    return -90;
}

double InputField::west() const {
    return 0;
}

double InputField::east() const {
    return 359;
}

double InputField::westEastIncrement() const {
    return 1;
}

double InputField::southNorthIncrement() const {
    return 1;
}

std::vector<size_t> InputField::dimensions() const {
    return owner_.cube().dimensions();
}

void InputField::values(std::vector<double>& values) const {
    Matrix* m = owner_.matrix();
    std::cout << "MATRIX " << *m << std::endl;
    values.clear();
}

}
}
