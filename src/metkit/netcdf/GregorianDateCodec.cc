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

#include "metkit/netcdf/GregorianDateCodec.h"

#include "metkit/netcdf/Exceptions.h"
#include "metkit/netcdf/OutputAttribute.h"
#include "metkit/netcdf/Value.h"
#include "metkit/netcdf/Variable.h"

#include <ostream>
#include <sstream>
#include <algorithm>

#include <netcdf.h>
namespace metkit{
namespace netcdf{
static long long offset = 0;

GregorianDateCodec::GregorianDateCodec(const std::string &units, const std::string &calendar):
    reference_(units.substr(strlen("seconds since "))),
    zero_(0),
    units_(units),
    calendar_(calendar)

{
    offset_ = reference_.date().julian() * 24 * 60 * 60 + eckit::Second(reference_.time());
    if (offset == 0) { // Not thread safe
        offset = offset_;
    }
    offset_ -= offset;
}

GregorianDateCodec::~GregorianDateCodec() {

}

void GregorianDateCodec::print(std::ostream &out) const {
    out << "GregorianDateCodec[reference=" << reference_ << ", calendar=" << calendar_ << ", offset=" << offset_ << "]";
}

template<class T>
void GregorianDateCodec::_decode(std::vector<T> &v) const {
    for (size_t i = 0; i < v.size(); i++) {
        ASSERT(T(v[i] + offset_) - offset_ == v[i]);
        v[i] += offset_;
    }
}

void GregorianDateCodec::decode(std::vector<double> &v) const {
    _decode(v);
}

void GregorianDateCodec::decode(std::vector<float> &v) const {
    _decode(v);
}

void GregorianDateCodec::decode(std::vector<long> &v) const {
    _decode(v);
}

void GregorianDateCodec::decode(std::vector<short> &v) const {
    _decode(v);
}

void GregorianDateCodec::decode(std::vector<unsigned char> &v) const {
    _decode(v);
}

template<class T>
static T _encode(std::vector<T> &v) {
    ASSERT(v.size());
    T zero = *std::min_element(v.begin(), v.end());
    if (zero != v[0]) {
        for (size_t i = 0; i < v.size(); i++) {
            ASSERT(T(v[i] - zero) + zero == v[i]);
            v[i] -= zero;
        }
        return zero;
    }
    return 0;
}

void GregorianDateCodec::encode(std::vector<double> &v) const {
    zero_ = _encode(v);
}

void GregorianDateCodec::encode(std::vector<float> &v) const {
    zero_ = _encode(v);
}

void GregorianDateCodec::encode(std::vector<long> &v) const {
    zero_ = _encode(v);
}

void GregorianDateCodec::encode(std::vector<short> &v) const {
    zero_ = _encode(v);
}

void GregorianDateCodec::encode(std::vector<unsigned char> &v) const {
    zero_ = _encode(v);
}

void GregorianDateCodec::addAttributes(Variable &v) const {
    v.add(new OutputAttribute(v, "units", Value::newFromString("seconds since YYYY-MM-DD HH-MM-SS")));
    v.add(new OutputAttribute(v, "calendar", Value::newFromString(calendar_)));
}

void GregorianDateCodec::updateAttributes(int nc, int varid, const std::string &path) {
    std::stringstream s;
    eckit::DateTime dt = reference_ + eckit::Second(zero_);
    s << "seconds since " << dt.date() << " " << dt.time();
    std::string value = s.str();
    NC_CALL(nc_put_att_text(nc, varid, "units", value.size(), value.c_str()), path);
}

bool GregorianDateCodec::timeAxis() const {
    return true;
}
}
}
