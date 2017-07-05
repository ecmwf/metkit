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

#include "MergeCoordinateMatrix.h"

#include "Mapper.h"
#include "Type.h"

#include <ostream>

MergeCoordinateMatrix::MergeCoordinateMatrix(Matrix *out, Matrix *in, size_t size):
    Matrix(Type::lookup(out->type(), in->type()), out->name(), size),
    out_(out),
    in_(in)
{
    out_->attach();
    in_->attach();
}

MergeCoordinateMatrix::~MergeCoordinateMatrix() {
    out_->detach();
    in_->detach();
}

void MergeCoordinateMatrix::print(std::ostream &out) const {
    out << "MergeCoordinateMatrix[name=" << name_
        << ", ptr=" << this
        << ", size=" << size_
        << "]";
}

void MergeCoordinateMatrix::dumpTree(std::ostream &out, size_t depth) const {
    for (size_t i = 0; i < depth; i++) out << " ";
    out << *this;
    out << std::endl;
    out_->dumpTree(out, depth + 1);
    in_->dumpTree(out, depth + 1);
}

template<class T>
void MergeCoordinateMatrix::_fill(Mapper<T> &v) const {
    v.overlap(true);
    Mapper<T> omap(v, out_->reshape());
    Mapper<T> imap(v, in_->reshape());
    out_->fill(omap);
    in_->fill(imap);
    v.overlap(false);
}

void MergeCoordinateMatrix::fill(Mapper<double> &v) const {
    _fill(v);
}

void MergeCoordinateMatrix::fill(Mapper<float> &v) const {
    _fill(v);
}

void MergeCoordinateMatrix::fill(Mapper<long> &v) const {
    _fill(v);
}

void MergeCoordinateMatrix::fill(Mapper<short> &v) const {
    _fill(v);
}

void MergeCoordinateMatrix::fill(Mapper<unsigned char> &v) const {
    _fill(v);
}
