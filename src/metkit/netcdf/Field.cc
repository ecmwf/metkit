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

#include "Field.h"

#include "Attribute.h"
#include "Dimension.h"
#include "Exceptions.h"
#include "Variable.h"

#include <netcdf.h>


Field::Field(const std::string &path):
    path_(path)
{

}

Field::~Field()
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

void Field::add(Dimension *d) {
    dimensions_[d->name()] = d;
}

void Field::add(Variable *v) {
    // Note: this is 'ncname'
    variables_[v->ncname()] = v;
}


const std::map<std::string, Dimension *> &Field::dimensions() const {
    return dimensions_;
}

const std::map<std::string, Variable *> &Field::variables() const {
    return variables_;
}

const std::string &Field::path() const {
    return path_;
}

const std::string &Field::name() const {
    static const std::string empty;
    return empty;
}

int Field::varid() const {
    return NC_GLOBAL;
}


Dimension *Field::findDimension(int id) const
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

Dimension *Field::findDimension(const std::string &name) const
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


void Field::dump(std::ostream &out) const
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

std::vector<Variable *> Field::variablesForDimension(const Dimension &dim) const {
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



