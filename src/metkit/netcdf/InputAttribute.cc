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

#include "InputAttribute.h"

#include "Endowed.h"
#include "OutputAttribute.h"
#include "Value.h"

#include <ostream>

InputAttribute::InputAttribute(Endowed &owner, const std::string &name, Value *value):
    Attribute(owner, name, value)
{
}

InputAttribute::~InputAttribute()
{
}

void InputAttribute::clone(Endowed &owner) const {
    owner.add(new OutputAttribute(owner, name_, value_->clone()));
}

void InputAttribute::print(std::ostream &out) const {
    out << "InputAttribute[name=" << name_ << "]";
}
