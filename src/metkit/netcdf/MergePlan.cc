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

#include "MergePlan.h"

#include "Exceptions.h"
#include "Variable.h"

#include <iostream>

MergePlan::MergePlan(Field &field):
    field_(field)
{

}

MergePlan::~MergePlan() {
    for (std::vector<Step *>::iterator j = steps_.begin(); j != steps_.end(); ++j) {
        delete (*j);
    }
}

void MergePlan::add(Step *s) {
    for (std::vector<Step *>::iterator j = steps_.begin(); j != steps_.end(); ++j) {
        if ((*j)->merge(s)) {
            delete s;
            return;
        }
    }
    queue_.push(s);
    steps_.push_back(s);
}

void MergePlan::execute() {
    while (!queue_.empty()) {
        Step *s = queue_.top();
        queue_.pop();
        s->execute(*this);
    }
}

void MergePlan::link(const Variable &out, const Variable &in) {
    ASSERT(link_.find(&out) == link_.end());
    link_[&out] = &in;
}

const Variable &MergePlan::link(const Variable &out) {
    if (link_.find(&out) == link_.end()) {
        std::cout << "MergePlan::link cannot find: " << out << std::endl;
    }
    ASSERT(link_.find(&out) != link_.end());
    return *link_[&out];
}

Field &MergePlan::field() const {
    return field_;
}
