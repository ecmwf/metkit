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

#include "Remapping.h"

#include "Exceptions.h"

#include <iostream>

Remapping::Remapping(size_t s) {
    reserve(s);
    for (size_t i = 0; i < s; i++) {
        push_back(i);
    }
}

Remapping::~Remapping() {

}

void Remapping::print(std::ostream &out) const {
    size_t i = 0;
    for (HyperCube::Remapping::const_iterator j = begin(); j != end(); ++j, ++i) {

        std::cout << " (" << i << "->" << *j << ")";
        if (i == 9) {
            std::cout << " ...";
            break;
        }
    }
}

void Remapping::validate() const {
    for (size_t i = 1; i < size(); i++) {
        ASSERT((*this)[i - 1] < (*this)[i]);
    }
}
