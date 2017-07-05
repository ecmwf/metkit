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

#include "DummyMatrix.h"

#include "Type.h"
#include "Variable.h"

#include <ostream>

DummyMatrix::DummyMatrix(const Variable &v):
    Matrix(v.matrix()->type(), v.name(), v.matrix()->size()),
    other_(v.matrix()),
    merged_(false)
{
    other_->attach();
}

DummyMatrix::~DummyMatrix()
{
    other_->detach();
}

void DummyMatrix::print(std::ostream &out) const {
    out << "DummyMatrix[type=" << *type_ << ",name=" << name_ << ",other=" << *other_ << "]";
}

void DummyMatrix::fill(Mapper<double> &v) const
{
    if (!merged_) {
        other_->fill(v);
    }
}

void DummyMatrix::fill(Mapper<float> &v) const
{
    if (!merged_) {
        other_->fill(v);
    }
}

void DummyMatrix::fill(Mapper<long> &v) const
{
    if (!merged_) {
        other_->fill(v);
    }
}

void DummyMatrix::fill(Mapper<short> &v) const
{
    if (!merged_) {
        other_->fill(v);
    }
}

void DummyMatrix::fill(Mapper<unsigned char> &v) const
{
    if (!merged_) {
        other_->fill(v);
    }
}

Matrix *DummyMatrix::merged() {
    // If this is called, the dummy matrix has  been merged with another
    // We should not use 'other_'
    merged_ = true;
    return this;
}
