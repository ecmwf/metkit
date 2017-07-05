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

#ifndef metkit_netcdf_Remapping
#define metkit_netcdf_Remapping

#include "eckit/memory/Counted.h"
#include "metkit/netcdf/HyperCube.h"

namespace metkit{
namespace netcdf{

class Remapping : public HyperCube::Remapping, public eckit::Counted {

public:
    Remapping(size_t = 0);
    virtual ~Remapping();

    void validate() const;

public:

    // -- Members
    // -- Methods

    virtual void print(std::ostream &s) const;

    // - Friend
    friend std::ostream &operator<<(std::ostream &s, const Remapping &v)
    {
        v.print(s);
        return s;
    }
};

}
}
#endif
