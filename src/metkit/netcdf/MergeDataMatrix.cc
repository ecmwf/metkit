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

#include "metkit/netcdf/MergeDataMatrix.h"

#include "metkit/netcdf/Mapper.h"
#include "metkit/netcdf/Type.h"

#include <ostream>

namespace metkit{
namespace netcdf{

MergeDataMatrix::MergeDataMatrix(Matrix *out, Matrix *in, size_t size):
    Matrix(Type::lookup(out->type(), in->type()), out->name(), size),
    out_(out),
    in_(in)
{
    out_->attach();
    in_->attach();
}

MergeDataMatrix::~MergeDataMatrix() {
    out_->detach();
    in_->detach();
}

void MergeDataMatrix::print(std::ostream &out) const {
    out << "MergeDataMatrix[" << name() << "]";
}

void MergeDataMatrix::dumpTree(std::ostream &out, size_t depth) const {
    for (size_t i = 0; i < depth; i++) out << " ";
    out << *this;
    out << std::endl;
    out_->dumpTree(out, depth + 1);
    in_->dumpTree(out, depth + 1);
}

template<class T>
void MergeDataMatrix::_fill(Mapper<T> &v) const {
    Mapper<T> omap(v, out_->reshape());
    Mapper<T> imap(v, in_->reshape());
    out_->fill(omap);
    in_->fill(imap);
}

void MergeDataMatrix::fill(Mapper<double> &v) const {
    _fill(v);
}

void MergeDataMatrix::fill(Mapper<float> &v) const {
    _fill(v);
}

void MergeDataMatrix::fill(Mapper<long> &v) const {
    _fill(v);
}

void MergeDataMatrix::fill(Mapper<short> &v) const {
    _fill(v);
}

void MergeDataMatrix::fill(Mapper<unsigned char> &v) const {
    _fill(v);
}

}
}
