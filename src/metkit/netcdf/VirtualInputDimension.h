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

#ifndef metkit_netcdf_VirtualInputDimension
#define metkit_netcdf_VirtualInputDimension

#include "metkit/netcdf/Dimension.h"

namespace metkit{
namespace netcdf{

class VirtualInputDimension : public Dimension {
public:
    VirtualInputDimension(Dataset &owner, const std::string &name);
    virtual ~VirtualInputDimension();

private:

    // Members

    // -- Methods

    // From Dimension
    virtual void print(std::ostream &s) const;
    virtual void clone(Dataset &owner) const;
    virtual int id() const;
};

}
}
#endif
