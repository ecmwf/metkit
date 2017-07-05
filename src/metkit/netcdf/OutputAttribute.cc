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

#include "OutputAttribute.h"

#include "Endowed.h"
#include "Exceptions.h"
#include "Value.h"

#include <iostream>

/*
See http://www.unidata.ucar.edu/software/netcdf/docs/netcdf/Attribute-Conventions.html
*/

static const char *dont_drop[] = {
    "_FillValue",
    "missing_value",
    0
};

OutputAttribute::OutputAttribute(Endowed &owner, const std::string &name, Value *value):
    Attribute(owner, name, value),
    valid_(true)
{

}

OutputAttribute::~OutputAttribute()
{
}

void OutputAttribute::create(int nc) const {
    if (valid_) {
        value_->createAttribute(nc, owner_.varid(), name_, owner_.path());
    }
}

void OutputAttribute::clone(Endowed &owner) const {
    owner.add(new OutputAttribute(owner, name_, value_->clone()));
}


void OutputAttribute::merge(const Attribute &other)
{
    if (!value_->sameAs(other.value())) {
        if (valid_) {
            std::cout << "WARNING: dropping attribute " << fullName() << std::endl;
            valid_ = false;

            size_t i = 0;
            while (dont_drop[i]) {
                if (name_ == dont_drop[i]) {
                    throw MergeError(std::string("Attempt to drop attribute ") + name_);
                }
                i++;
            }

        }
    }
}

void OutputAttribute::invalidate() {
    valid_ = false;
}


void OutputAttribute::print(std::ostream &out) const {
    out << "OutputAttribute[name=" << name_ << "]";
}
