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

#ifndef metkit_netcdf_Field
#define metkit_netcdf_Field

#include <iosfwd>

#include "eckit/memory/ScopedPtr.h"


namespace metkit {
namespace netcdf {

class Variable;
class GridSpec;

class Field  {
public:

    Field(const Variable &);
    virtual ~Field();

    // -- Methods

    // Same as GRIB


    // For MIR
    bool has(const std::string& name) const;
    bool get(const std::string&, long&) const;
    bool get(const std::string&, std::string&) const;
    bool get(const std::string &name, double &value) const;

protected:

    // -- Members
    const Variable& variable_;

private:

    Field(const Field &);
    Field &operator=(const Field &);



    mutable eckit::ScopedPtr<GridSpec> gridSpec_;

    // - Methods

    const GridSpec &gridSpec() const;


    virtual void print(std::ostream &s) const = 0;

    // -- Friends
    friend std::ostream &operator<<(std::ostream &s, const Field &v) {
        v.print(s);
        return s;
    }
};

}
}
#endif
