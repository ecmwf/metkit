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

#ifndef metkit_netcdf_UpdateCoordinateStep
#define metkit_netcdf_UpdateCoordinateStep

#include "Step.h"

namespace metkit{
namespace netcdf{

class Variable;


class UpdateCoordinateStep : public Step  {
public:

    UpdateCoordinateStep(Variable &out, const Variable &in, size_t growth);
    ~UpdateCoordinateStep();

private:

    // Members

    Variable &out_;
    const Variable &in_;
    size_t growth_;

    // -- Methods
    virtual void print(std::ostream &out) const;
    virtual int rank() const;
    virtual void execute(MergePlan &plan);

};

}
}
#endif
