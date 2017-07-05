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

#ifndef metkit_netcdf_InputAttribute
#define metkit_netcdf_InputAttribute

#include "Attribute.h"

namespace metkit{
namespace netcdf{

class InputAttribute : public Attribute
{
public:

    InputAttribute(Endowed &owner, const std::string &name, Value *value);
    virtual ~InputAttribute();

private:

    // From Atttribute
    virtual void print(std::ostream &out) const;
    virtual void clone(Endowed &owner) const;

};

}
}
#endif
