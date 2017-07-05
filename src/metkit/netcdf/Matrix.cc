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

#include "Matrix.h"

#include "Codec.h"
#include "Exceptions.h"
#include "Mapper.h"
#include "MergeCoordinateMatrix.h"
#include "MergeDataMatrix.h"
#include "Reshape.h"
#include "Type.h"
#include "Value.h"

#include <iostream>
namespace metkit{
namespace netcdf{

Matrix::Matrix(Type &type, const std::string &name, size_t size):
    type_(&type),
    missingValue_(0),
    codec_(0),
    name_(name),
    size_(size)
{
}

Matrix::~Matrix() {
    delete missingValue_;

    for (std::vector<Reshape *>::iterator j = reshapes_.begin(); j != reshapes_.end(); ++j) {
        (*j)->detach();
    }

    if (codec_) {
        codec_->detach();
    }
}


Type &Matrix::type() const {
    return *type_;
}

void Matrix::type(Type &type) {
    type_ = &type;
}


const std::string &Matrix::name() const {
    return name_;
}

Matrix *Matrix::mergeData(Matrix *other, size_t size) {
    Matrix *m = new MergeDataMatrix(this->merged(), other->merged(), size);
    m->codec(codec_);
    return m;
}

Matrix *Matrix::mergeCoordinates(Matrix *other, size_t size)  {
    Matrix *m = new MergeCoordinateMatrix(this->merged(), other->merged(), size);
    m->codec(codec_);
    return m;
}

void Matrix::dump(std::ostream &out) const {
    type_->dump(out, *this);
}

void Matrix::dumpTree(std::ostream &out, size_t depth) const {
    for (size_t i = 0; i < depth; i++) out << "   ";
    out << *this;
    out << std::endl;
}

void Matrix::save(int nc, int varid, const std::string &path) const {
    type_->save(*this, nc, varid, path);
}

void Matrix::printValues(std::ostream &out) const {
    type_->printValues(out, *this);
}

void Matrix::missingValue(Value *value) {
    delete missingValue_;
    missingValue_ = value;
}

template<class T>
void Matrix::_fill(std::vector<T> &v) const {

    if (missingValue_) {
        missingValue_->init(v, size());
    }
    else {
        std::vector<T> w(size());
        std::swap(v, w);
    }

    std::vector<bool> set(size(), false);
    bool overlap = false;
    Mapper<T> mapper(v, set, overlap);

    fill(mapper);
}

void Matrix::values(std::vector<double> &v) const {
    _fill(v);
}

void Matrix::values(std::vector<float> &v) const {
    _fill(v);
}

void Matrix::values(std::vector<long> &v) const {
    _fill(v);
}

void Matrix::values(std::vector<short> &v) const {
    _fill(v);
}

void Matrix::values(std::vector<unsigned char> &v) const {
    _fill(v);
}

void Matrix::values(std::vector<std::string> &v) const {
    NOTIMP;
}

void Matrix::fill(Mapper<double> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Matrix::fill(Mapper<float> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Matrix::fill(Mapper<long> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Matrix::fill(Mapper<short> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

void Matrix::fill(Mapper<unsigned char> &) const {
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
}

size_t Matrix::size() const {
    return size_;
}

Matrix *Matrix::merged() {
    return this;
}

void Matrix::codec(Codec *codec) {
    decache();

    ASSERT(!codec_);
    if (codec) {
        codec->attach();
    }
    if (codec_) {
        codec_->detach();
    }
    codec_ = codec;
}

void Matrix::reshape(Reshape *r) {
    decache();
    if (reshapes_.size()) {
        Reshape *s = reshapes_.back();
        if (s->merge(*r)) {
            r->attach();
            r->detach();
            return;
        }
    }

    r->attach();
    std::cout << "Matrix::reshape " << name() << " " << *r << std::endl;
    reshapes_.push_back(r);
}

const std::vector<Reshape *> &Matrix::reshape() const {
    return reshapes_;
}

Codec *Matrix::codec() const {
    return codec_;
}

template<>
const std::vector<double> &Matrix::cache<double>() const {
    return double_cache_;
}

template<>
const std::vector<short> &Matrix::cache<short>() const {
    return short_cache_;
}

template<>
const std::vector<long> &Matrix::cache<long>() const {
    return long_cache_;
}

template<>
const std::vector<float> &Matrix::cache<float>() const {
    return float_cache_;
}

template<>
const std::vector<unsigned char> &Matrix::cache<unsigned char>() const {
    return unsigned_char_cache_;
}

template<>
const std::vector<std::string> &Matrix::cache<std::string>() const {
    return string_cache_;
}

void Matrix::decache() const {
    double_cache_.clear();
    float_cache_.clear();
    long_cache_.clear();
    short_cache_.clear();
    unsigned_char_cache_.clear();
    string_cache_.clear();
}
}
}
