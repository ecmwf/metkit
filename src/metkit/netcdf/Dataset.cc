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

#include "metkit/netcdf/Dataset.h"

#include "metkit/netcdf/Attribute.h"
#include "metkit/netcdf/Dimension.h"
#include "metkit/netcdf/Exceptions.h"
#include "metkit/netcdf/Variable.h"

#include <netcdf.h>

namespace metkit {
namespace netcdf {

Dataset::Dataset(const std::string &path):
    path_(path)
{

}

Dataset::~Dataset()
{
    for (std::map<std::string, Dimension *>::iterator j = dimensions_.begin(); j != dimensions_.end(); ++j)
    {
        delete (*j).second;
    }

    for (std::map<std::string, Variable *>::iterator j = variables_.begin(); j != variables_.end(); ++j)
    {
        delete (*j).second;
    }
}

void Dataset::add(Dimension *d) {
    dimensions_[d->name()] = d;
}

void Dataset::add(Variable *v) {
    // Note: this is 'ncname'
    variables_[v->ncname()] = v;
}


const std::map<std::string, Dimension *> &Dataset::dimensions() const {
    return dimensions_;
}

const std::map<std::string, Variable *> &Dataset::variables() const {
    return variables_;
}

const std::string &Dataset::path() const {
    return path_;
}

const std::string &Dataset::name() const {
    static const std::string empty;
    return empty;
}

int Dataset::varid() const {
    return NC_GLOBAL;
}


Dimension *Dataset::findDimension(int id) const
{
    for (std::map<std::string, Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j)
    {
        if ((*j).second->id() == id)
        {
            return (*j).second;
        }
    }
    ASSERT(0);
    return 0;
}

Dimension *Dataset::findDimension(const std::string &name) const
{
    for (std::map<std::string, Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j)
    {
        if ((*j).second->name() == name)
        {
            return (*j).second;
        }
    }
    ASSERT(0);
    return 0;
}


void Dataset::dump(std::ostream &out) const
{

    out << "netcdf " << path_ << "{" << std::endl;
    out << "dimensions:" << std::endl;
    for (std::map<std::string, Dimension *>::const_iterator j = dimensions_.begin(); j != dimensions_.end(); ++j)
    {
        (*j).second->dump(out);
    }
    out << "variables:" << std::endl;
    for (std::map<std::string, Variable *>::const_iterator j = variables_.begin(); j != variables_.end(); ++j)
    {
        (*j).second->dump(out);
    }
    out << "// global attributes:" << std::endl;
    for (std::map<std::string, Attribute *>::const_iterator j = attributes_.begin(); j != attributes_.end(); ++j)
    {
        (*j).second->dump(out);
    }

    out << std::endl << "data:" << std::endl;
    for (std::map<std::string, Variable *>::const_iterator j = variables_.begin(); j != variables_.end(); ++j)
    {
        (*j).second->dumpData(out);
    }
    out << std::endl << "}" << std::endl;
}

std::vector<Variable *> Dataset::variablesForDimension(const Dimension &dim) const {
    std::vector<Variable *> result;
    for (std::map<std::string, Variable *>::const_iterator j = variables_.begin(); j != variables_.end(); ++j)
    {
        std::vector<Dimension *> dimensions = (*j).second->dimensions();
        for (std::vector<Dimension *>::iterator k = dimensions.begin(); k != dimensions.end(); ++k) {
            if ((*k) == &dim) {
                result.push_back((*j).second);
                break;
            }
        }
    }
    return result;
}

//======================================================
const Variable& Dataset::variable(const std::string& name) const {
    std::map<std::string, Variable *>::const_iterator j = variables_.find(name);
    if (j == variables_.end()) {
        throw eckit::UserError("Cannot find netcdf variable '" + name + "'");
    }
    return *(*j).second;
}


}
}
