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

#ifndef metkit_netcdf_InputVariable
#define metkit_netcdf_InputVariable

#include "Variable.h"


namespace metkit{
namespace netcdf{

class InputVariable : public Variable {
public:

    InputVariable(Field &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions);
    virtual ~InputVariable();

protected:

    // Members
    int id_;

    // -- Methods

    virtual Variable *makeOutputVariable(Field &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const = 0;

    // From variable
    virtual void print(std::ostream &s) const;
    virtual Variable *clone(Field &owner) const;

    // From Endowed

    virtual int varid() const;
};

}
}
#endif
