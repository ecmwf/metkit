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

#ifndef MergeCoordinateStep_H
#define MergeCoordinateStep_H

#include "Step.h"

class Variable;

class MergeCoordinateStep : public Step  {
public:

    MergeCoordinateStep( Variable &out, const Variable &in);
    ~MergeCoordinateStep();

private:

    // Members

    Variable &out_;
    const Variable &in_;

    // -- Methods
    virtual void print(std::ostream &out) const;
    virtual int rank() const;
    virtual void execute(MergePlan &plan);
};

#endif
