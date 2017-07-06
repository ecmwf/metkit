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

#ifndef metkit_netcdf_DataInputVariable
#define metkit_netcdf_DataInputVariable

#include "metkit/netcdf/InputVariable.h"


namespace metkit{
namespace netcdf{

class DataInputVariable : public InputVariable {
public:
    DataInputVariable(Dataset &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions);
    virtual ~DataInputVariable();
private:

    Variable *makeOutputVariable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const ;
    virtual void print(std::ostream &s) const;
    virtual const std::string &ncname() const;

    mutable std::string ncname_;

};

}
}
#endif
