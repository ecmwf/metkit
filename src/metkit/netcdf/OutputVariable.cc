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

#include "OutputVariable.h"

#include "Attribute.h"
#include "Dimension.h"
#include "Exceptions.h"
#include "Field.h"
#include "Matrix.h"
#include "Type.h"
#include "Codec.h"

#include <netcdf.h>

namespace metkit{
namespace netcdf{
OutputVariable::OutputVariable(Field &owner, const std::string &name, const std::vector<Dimension *> &dimensions):
    Variable(owner, name, dimensions),
    created_(false),
    id_(-1)
{
}

OutputVariable::~OutputVariable()
{
}


void OutputVariable::create(int nc) const {

    ASSERT(!created_);
    ASSERT(matrix_ != 0);

    int dims[NC_MAX_VAR_DIMS];
    int ndims = 0;
    for (std::vector<Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j) {
        if ((*j)->inUse()) {
            dims[ndims++] = (*j)->id();
        }
    }

    std::string name = ncname();

    NC_CALL(nc_def_var(nc, name.c_str(), matrix_->type().code(), ndims, dims, &id_), owner_.path());

    Codec *codec = matrix_->codec();
    if (codec) {
        Variable *self = const_cast<OutputVariable *>(this);
        codec->addAttributes(*self);
    }

    created_ = true;

    for (std::map<std::string, Attribute *>::const_iterator j = attributes_.begin(); j != attributes_.end(); ++j)
    {
        (*j).second->create(nc);
    }
}

void OutputVariable::save(int nc) const {
    ASSERT(created_);
    matrix_->save(nc, id_, path());

    Codec *codec = matrix_->codec();
    if (codec) {
        codec->updateAttributes(nc, id_, path());
    }
}

void OutputVariable::print(std::ostream &out) const {
    out << "OutputVariable[name=" << name_ << "]";
}

int OutputVariable::varid() const {
    ASSERT(created_);
    ASSERT(id_ >= 0);
    return id_;
}

}
}
