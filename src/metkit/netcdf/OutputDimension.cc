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

#include "OutputDimension.h"

#include "Exceptions.h"
#include "Field.h"

#include <netcdf.h>

OutputDimension::OutputDimension(Field &owner, const std::string &name, size_t len):
    Dimension(owner, name, len),
    id_(-1),
    created_(false),
    grown_(false)
{

}

OutputDimension::~OutputDimension()
{
}

void OutputDimension::create(int nc) const {
    ASSERT(!created_);
    NC_CALL(nc_def_dim(nc, name_.c_str(), len_, &id_), owner_.path());
    created_ = true;
}

void OutputDimension::print(std::ostream &out) const {
    out << "OutputDimension[name=" << name_ << ",size=" << len_ << "]";
}

int OutputDimension::id() const {
    ASSERT(created_);
    return id_;
}

void OutputDimension::grow(size_t count) {
    ASSERT(!created_);
    ASSERT(count >= len_);
    len_ = count;
    grown_ = true;
}
