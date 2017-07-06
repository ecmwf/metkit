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

#ifndef metkit_netcdf_InputField
#define metkit_netcdf_InputField

#include "metkit/netcdf/Field.h"

#include <string>
#include <vector>

namespace metkit {
namespace netcdf {

class DataInputVariable;

class InputField : public Field {
public:

    InputField(const DataInputVariable &);
    virtual ~InputField();

    // -- Methods


    // From Endowed


protected:

    // -- Members
    const DataInputVariable & owner_;

private:

    InputField(const InputField &);
    InputField &operator=(const InputField &);

    // From Endowed

    virtual std::string gridType() const;
    virtual long paramId() const;
    virtual double north() const;
    virtual double south() const;
    virtual double west() const;
    virtual double east() const;
    virtual double westEastIncrement() const;
    virtual double southNorthIncrement() const;
    virtual std::vector<size_t> dimensions() const;
    virtual void values(std::vector<double>&) const;


    // - Methods

    virtual void print(std::ostream &s) const;

};

}
}
#endif
