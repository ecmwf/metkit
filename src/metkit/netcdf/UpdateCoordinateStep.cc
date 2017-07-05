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

#include "UpdateCoordinateStep.h"

#include "Exceptions.h"
#include "Field.h"
#include "MergeCoordinateStep.h"
#include "MergePlan.h"
#include "ReshapeVariableStep.h"
#include "Variable.h"

#include <iostream>


UpdateCoordinateStep::UpdateCoordinateStep( Variable &out, const Variable &in, size_t growth):
    out_(out),
    in_(in),
    growth_(growth)
{
}

UpdateCoordinateStep::~UpdateCoordinateStep() {

}

int UpdateCoordinateStep::rank() const {
    return 1;
}

void UpdateCoordinateStep::print(std::ostream &out) const {
    out << "UpdateCoordinateStep[" << in_ << " & " << out_ << "]";
}

void UpdateCoordinateStep::execute(MergePlan &plan) {
    const std::vector<Dimension *> &dims = out_.dimensions();
    ASSERT(dims.size() == 1);

    std::cout << *this << std::endl;

    std::vector<Variable *> v = plan.field().variablesForDimension(*dims[0]);
    for (std::vector<Variable *>::iterator j = v.begin(); j != v.end(); ++j) {
        std::cout << "Affects: " << **j << std::endl;
        (*j)->mustMerge(true);
        plan.add(new ReshapeVariableStep(**j, *dims[0], growth_));
    }

    plan.add(new MergeCoordinateStep(out_, in_));
}
