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

#include "metkit/netcdf/Field.h"

#include "metkit/netcdf/GridSpec.h"

#include <iostream>

namespace metkit {
namespace netcdf {

Field::Field(const Variable &variable):
    variable_(variable) {
}

Field::~Field()
{
}

const GridSpec &Field::gridSpec() const {
    if (!gridSpec_) {
        // TODO: may need a mutex
        gridSpec_.reset(GridSpec::create(variable_));
        std::cout << *gridSpec_ << std::endl;
    }
    return *gridSpec_;
}

bool Field::has(const std::string& name) const {
    return gridSpec().has(name);
}

bool Field::get(const std::string&name, long& value) const {
    if (name == "paramId") {
        value = 1;
        return true;
    }
    return gridSpec().get(name, value);
}

bool Field::get(const std::string&name, std::string& value) const {
    return gridSpec().get(name, value);
}

bool Field::get(const std::string &name, double &value) const {
    return gridSpec().get(name, value);
}

}
}
