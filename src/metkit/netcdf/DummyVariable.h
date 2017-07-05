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

#ifndef metkit_netcdf_DummyVariable
#define metkit_netcdf_DummyVariable

#include "metkit/netcdf/Variable.h"


namespace metkit {
namespace netcdf {

class DummyVariable : public Variable {
public:

    DummyVariable(const Variable &parent);
    virtual ~DummyVariable();

protected:

    const Variable &parent_;

    // -- Methods

    virtual Variable *makeOutputVariable(Field &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const;
    virtual bool dummy() const;
    virtual bool sameAsDummy(const Variable &) const;

    // From variable

    virtual void print(std::ostream &s) const;

    // From Endowed

    virtual int varid() const;
};

}
}
#endif
