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

#include "ReshapeVariableStep.h"

#include "Dimension.h"
#include "Exceptions.h"
#include "Matrix.h"
#include "MergePlan.h"
#include "Remapping.h"
#include "Reshape.h"
#include "Variable.h"

#include <iostream>

ReshapeVariableStep::ReshapeVariableStep(Variable &out, const Dimension &dimension, size_t growth):
    out_(out),
    dimension_(dimension),
    growth_(growth),
    next_(0)
{

}

ReshapeVariableStep::~ReshapeVariableStep() {
    delete next_;
}

int ReshapeVariableStep::rank() const {
    return 2;
}

void ReshapeVariableStep::print(std::ostream &out) const {
    out << "ReshapeVariableStep[" << out_ << ", dim=" << dimension_;

    if (next_) {
        out << ", next=" << *next_;
    }
    out << "]";
}

void ReshapeVariableStep::execute(MergePlan &plan) {
    std::cout << "ReshapeVariableStep::execute(): " << out_ << std::endl;

    const std::vector<Dimension *> &dims = out_.dimensions();

    HyperCube &cube = out_.cube();

    std::vector<Dimension *> dimensions; // Dimension to reshape, in reverse variable order
    std::vector<size_t> growth;
    std::vector<size_t> indexes;

    size_t idx = dims.size() - 1;
    for (std::vector<Dimension *>::const_reverse_iterator j = dims.rbegin(); j != dims.rend(); ++j, --idx) {
        ReshapeVariableStep *v = this;
        while (v) {
            if (&v->dimension_ == *j) {
                dimensions.push_back(*j);
                growth.push_back(v->growth_);
                indexes.push_back(idx);
            }
            v = v->next_;
        }
    }

    const Variable &in = plan.link(out_);
    HyperCube incube = in.cube();

    size_t i = 0;

    ASSERT(cube.size() == incube.size());

    for (std::vector<Dimension *>::const_iterator j = dimensions.begin(); j != dimensions.end(); ++j, ++i) {

        // Add at end
        size_t where = (*j)->count();
        size_t idx = indexes[i];

        const Remapping &dimremap = (*j)->remapping();
        std::cout << "ReshapeVariableStep::execute() - dimremap " << (*j)->name() << ": " << dimremap << std::endl;

        size_t gap = 0;
        for (size_t k = 0; k < dimremap.size(); k++) {
            size_t g = dimremap[k] - k;
            if (g != gap) {
                std::cout << "ReshapeVariableStep::execute() - where=" << k << " gap=" << g << std::endl;

                in.matrix()->reshape(new Reshape(incube, idx, k, g - gap, 'I'));

                incube.dimensions()[idx] += g - gap;
                gap += g;
            }

        }

        if (growth[i]) {
            out_.matrix()->reshape(new Reshape(cube, idx, where, growth[i], 'O'));
            cube.dimensions()[idx] += growth[i];
        }

        if (incube.dimensions(idx) < cube.dimensions(idx)) {
            in.matrix()->reshape(new Reshape(incube, idx,
                                             incube.dimensions(idx), cube.dimensions(idx) - incube.dimensions(idx), 'F'));
            incube.dimensions()[idx] += cube.dimensions(idx) - incube.dimensions(idx);
        }

    }


    std::cout << "ReshapeVariableStep::execute() - Remapping out " << std::endl;
    const std::vector<Reshape *> a = out_.matrix()->reshape();
    for (std::vector<Reshape *>::const_iterator j = a.begin(); j != a.end(); ++j) std::cout << *(*j) << std::endl;

    std::cout << "ReshapeVariableStep::execute() - Remapping in " << std::endl;
    const std::vector<Reshape *> b = in.matrix()->reshape();
    for (std::vector<Reshape *>::const_iterator j = b.begin(); j != b.end(); ++j) std::cout << *(*j) << std::endl;
}

bool ReshapeVariableStep::merge(Step *other) {
    ReshapeVariableStep *o = dynamic_cast<ReshapeVariableStep *>(other);
    if (o) {
        // Same variable
        if (&(o->out_) == &(out_)) {

            ReshapeVariableStep *next = new ReshapeVariableStep(o->out_, o->dimension_, o->growth_);
            ReshapeVariableStep *prev = 0;
            ReshapeVariableStep *self = this;

            while (self) {
                prev = self;
                self = self->next_;
            }

            if (prev) {
                prev->next_ = next;
            }
            else {
                next_ = next;
            }

            return true;
        }
    }
    return false;
}
