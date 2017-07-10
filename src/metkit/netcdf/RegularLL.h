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

#ifndef metkit_netcdf_RegularLL
#define metkit_netcdf_RegularLL

#include "metkit/netcdf/GridSpec.h"
#include "eckit/exception/Exceptions.h"


namespace metkit {
namespace netcdf {


class RegularLL : public GridSpec {
public:

    RegularLL(const Variable &,
              double north,
              double south,
              double south_north_increment,
              double west,
              double east,
              double west_east_increment);



    virtual ~RegularLL();

    // -- Methods


    static GridSpec* guess(const Variable &variable,
                           const Variable &latitudes,
                           const Variable &longitudes);


protected:

    // -- Members

    double north_;
    double south_;
    double south_north_increments_;

    double west_;
    double east_;
    double west_east_increment_;


private:

    RegularLL(const RegularLL &);
    RegularLL &operator=(const RegularLL &);


    // - Methods

    virtual void print(std::ostream &s) const;

    // For MIR
    virtual bool has(const std::string& name) const;
    virtual bool get(const std::string&, long&) const;
    virtual bool get(const std::string&, std::string&) const;
    virtual bool get(const std::string &name, double &value) const;

};

}
}
#endif
