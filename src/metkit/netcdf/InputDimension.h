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

#ifndef metkit_netcdf_InputDimension
#define metkit_netcdf_InputDimension

#include "Dimension.h"

namespace metkit{
namespace netcdf{

class InputDimension : public Dimension {
public:
    InputDimension(Field &owner, const std::string &name, int id, size_t len);
    virtual ~InputDimension();

private:

    // Members

    int id_;

    // -- Methods

    // From Dimension
    virtual void print(std::ostream &s) const;
    virtual void clone(Field &owner) const;
    virtual int id() const;
};

}
}
#endif
