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

#include "Value.h"

#include "Exceptions.h"
#include "Type.h"

#include <netcdf.h>


template<>
void ValueT<unsigned char>::createAttribute(int nc, int varid, const std::string &name, const std::string &path) {
    NC_CALL(nc_put_att_ubyte(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}

template<>
void ValueT<unsigned char>::dump(std::ostream &out) const {
    out << value_;
}

template<>
void ValueT<short>::createAttribute(int nc, int varid, const std::string &name, const std::string &path) {
    NC_CALL(nc_put_att_short(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}

template<>
void ValueT<short>::dump(std::ostream &out) const {
    out << value_;
}

template<>
void ValueT<long>::createAttribute(int nc, int varid, const std::string &name, const std::string &path) {
    NC_CALL(nc_put_att_long(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}

template<>
void ValueT<long>::dump(std::ostream &out) const {
    out << value_;
}

template<>
void ValueT<std::string>::dump(std::ostream &out) const {
    out << '"' << value_ << '"';
}

template<>
void ValueT<std::string>::createAttribute(int nc, int varid, const std::string &name, const std::string &path) {
    NC_CALL(nc_put_att_text(nc, varid, name.c_str(), value_.size(), value_.c_str()), path);
}

template<>
void ValueT<float>::createAttribute(int nc, int varid, const std::string &name, const std::string &path) {
    NC_CALL(nc_put_att_float(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}


template<>
void ValueT<float>::dump(std::ostream &out) const {
    out << value_;
}

template<>
void ValueT<double>::createAttribute(int nc, int varid, const std::string &name, const std::string &path) {
    NC_CALL(nc_put_att_double(nc, varid, name.c_str(), type_.code(), 1, &value_), path);
}


template<>
void ValueT<double>::dump(std::ostream &out) const {
    out << value_;
}


//=============================
template<>
std::string ValueT<std::string>::asString() const {
    return value_;
}

template<>
std::string ValueT<double>::asString() const {
    NOTIMP; return "";
}

template<>
std::string ValueT<float>::asString() const {
    NOTIMP; return "";
}

template<>
std::string ValueT<unsigned char>::asString() const {
    NOTIMP; return "";
}

template<>
std::string ValueT<long>::asString() const {
    NOTIMP; return "";
}

template<>
std::string ValueT<short>::asString() const {
    NOTIMP; return "";
}

//=============================
template<>
void ValueT<unsigned char>::fill(const std::vector<bool> &set, std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::fill(const std::vector<bool> &set, std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::fill(const std::vector<bool> &set, std::vector<unsigned char> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (!set[i]) {
            v[i] = value_;
        }
    }
}
template<>
void ValueT<unsigned char>::fill(const std::vector<bool> &set, std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::fill(const std::vector<bool> &set, std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::fill(const std::vector<bool> &set, std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<short>::fill(const std::vector<bool> &set, std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::fill(const std::vector<bool> &set, std::vector<short> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (!set[i]) {
            v[i] = value_;
        }
    }
}
template<>
void ValueT<short>::fill(const std::vector<bool> &set, std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::fill(const std::vector<bool> &set, std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::fill(const std::vector<bool> &set, std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::fill(const std::vector<bool> &set, std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<long>::fill(const std::vector<bool> &set, std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::fill(const std::vector<bool> &set, std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::fill(const std::vector<bool> &set, std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::fill(const std::vector<bool> &set, std::vector<long> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (!set[i]) {
            v[i] = value_;
        }
    }
}
template<>
void ValueT<long>::fill(const std::vector<bool> &set, std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::fill(const std::vector<bool> &set, std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<float>::fill(const std::vector<bool> &set, std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::fill(const std::vector<bool> &set, std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::fill(const std::vector<bool> &set, std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::fill(const std::vector<bool> &set, std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::fill(const std::vector<bool> &set, std::vector<float> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (!set[i]) {
            v[i] = value_;
        }
    }
}
template<>
void ValueT<float>::fill(const std::vector<bool> &set, std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<double>::fill(const std::vector<bool> &set, std::vector<double> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (!set[i]) {
            v[i] = value_;
        }
    }
}

template<>
void ValueT<double>::fill(const std::vector<bool> &set, std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::fill(const std::vector<bool> &set, std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::fill(const std::vector<bool> &set, std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::fill(const std::vector<bool> &set, std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::fill(const std::vector<bool> &set, std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<std::string>::fill(const std::vector<bool> &set, std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::fill(const std::vector<bool> &set, std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::fill(const std::vector<bool> &set, std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::fill(const std::vector<bool> &set, std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::fill(const std::vector<bool> &set, std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::fill(const std::vector<bool> &set, std::vector<std::string> &v) {
    NOTIMP;
}

//=============================
template<>
void ValueT<unsigned char>::clear( std::vector<bool> &set, const std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::clear( std::vector<bool> &set, const std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::clear( std::vector<bool> &set, const std::vector<unsigned char> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (v[i] == value_) {
            set[i] = false;
        }
    }
}
template<>
void ValueT<unsigned char>::clear( std::vector<bool> &set, const std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::clear( std::vector<bool> &set, const std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<unsigned char>::clear( std::vector<bool> &set, const std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<short>::clear( std::vector<bool> &set, const std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::clear( std::vector<bool> &set, const std::vector<short> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (v[i] == value_) {
            set[i] = false;
        }
    }
}
template<>
void ValueT<short>::clear( std::vector<bool> &set, const std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::clear( std::vector<bool> &set, const std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::clear( std::vector<bool> &set, const std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<short>::clear( std::vector<bool> &set, const std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<long>::clear( std::vector<bool> &set, const std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::clear( std::vector<bool> &set, const std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::clear( std::vector<bool> &set, const std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::clear( std::vector<bool> &set, const std::vector<long> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (v[i] == value_) {
            set[i] = false;
        }
    }
}
template<>
void ValueT<long>::clear( std::vector<bool> &set, const std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<long>::clear( std::vector<bool> &set, const std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<float>::clear( std::vector<bool> &set, const std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::clear( std::vector<bool> &set, const std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::clear( std::vector<bool> &set, const std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::clear( std::vector<bool> &set, const std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<float>::clear( std::vector<bool> &set, const std::vector<float> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (v[i] == value_) {
            set[i] = false;
        }
    }
}
template<>
void ValueT<float>::clear( std::vector<bool> &set, const std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<double>::clear( std::vector<bool> &set, const std::vector<double> &v) {
    for (size_t i = 0; i < set.size(); i++) {
        if (v[i] == value_) {
            set[i] = false;
        }
    }
}

template<>
void ValueT<double>::clear( std::vector<bool> &set, const std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::clear( std::vector<bool> &set, const std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::clear( std::vector<bool> &set, const std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::clear( std::vector<bool> &set, const std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<double>::clear( std::vector<bool> &set, const std::vector<std::string> &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<std::string>::clear( std::vector<bool> &set, const std::vector<double> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::clear( std::vector<bool> &set, const std::vector<short> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::clear( std::vector<bool> &set, const std::vector<unsigned char> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::clear( std::vector<bool> &set, const std::vector<long> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::clear( std::vector<bool> &set, const std::vector<float> &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::clear( std::vector<bool> &set, const std::vector<std::string> &v) {
    NOTIMP;
}


Value *Value::newFromString(const std::string &s) {
    return new ValueT<std::string>(Type::lookup(NC_CHAR), s);
}

//=============================
template<>
void ValueT<unsigned char>::init(std::vector<double> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::init(std::vector<short> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::init(std::vector<unsigned char> &v, size_t size) {
    std::vector<unsigned char> w(size, value_);
    std::swap(v, w);
}

template<>
void ValueT<unsigned char>::init(std::vector<long> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::init(std::vector<float> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::init(std::vector<std::string> &v, size_t size) {
    NOTIMP;
}
//=============================
template<>
void ValueT<short>::init(std::vector<double> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<short>::init(std::vector<short> &v, size_t size) {
    std::vector<short> w(size, value_);
    std::swap(v, w);
}

template<>
void ValueT<short>::init(std::vector<unsigned char> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<short>::init(std::vector<long> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<short>::init(std::vector<float> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<short>::init(std::vector<std::string> &v, size_t size) {
    NOTIMP;
}
//=============================
template<>
void ValueT<long>::init(std::vector<double> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<long>::init(std::vector<short> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<long>::init(std::vector<long> &v, size_t size) {
    std::vector<long> w(size, value_);
    std::swap(v, w);
}

template<>
void ValueT<long>::init(std::vector<unsigned char> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<long>::init(std::vector<float> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<long>::init(std::vector<std::string> &v, size_t size) {
    NOTIMP;
}
//=============================
template<>
void ValueT<float>::init(std::vector<double> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<float>::init(std::vector<short> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<float>::init(std::vector<unsigned char> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<float>::init(std::vector<long> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<float>::init(std::vector<float> &v, size_t size) {
    std::vector<float> w(size, value_);
    std::swap(v, w);
}

template<>
void ValueT<float>::init(std::vector<std::string> &v, size_t size) {
    NOTIMP;
}

//=============================
template<>
void ValueT<double>::init(std::vector<double> &v, size_t size) {
    std::vector<double> w(size, value_);
    std::swap(v, w);
}

template<>
void ValueT<double>::init(std::vector<short> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<double>::init(std::vector<unsigned char> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<double>::init(std::vector<long> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<double>::init(std::vector<float> &v, size_t size) {
    NOTIMP;
}

template<>
void ValueT<double>::init(std::vector<std::string> &v, size_t size) {
    NOTIMP;
}
//=============================
template<>
void ValueT<std::string>::init(std::vector<double> &v, size_t size) {
    NOTIMP;
}
template<>
void ValueT<std::string>::init(std::vector<short> &v, size_t size) {
    NOTIMP;
}
template<>
void ValueT<std::string>::init(std::vector<unsigned char> &v, size_t size) {
    NOTIMP;
}
template<>
void ValueT<std::string>::init(std::vector<long> &v, size_t size) {
    NOTIMP;
}
template<>
void ValueT<std::string>::init(std::vector<float> &v, size_t size) {
    NOTIMP;
}
template<>
void ValueT<std::string>::init(std::vector<std::string> &v, size_t size) {
    NOTIMP;
}

//=============================
//=============================
template<>
void ValueT<unsigned char>::get(double &v) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::get(short &v) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::get(unsigned char &v) {
    v = value_;
}

template<>
void ValueT<unsigned char>::get(long &v) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::get(float &v) {
    NOTIMP;
}

template<>
void ValueT<unsigned char>::get(std::string &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<short>::get(double &v) {
    NOTIMP;
}

template<>
void ValueT<short>::get(short &v) {
    v = value_;
}

template<>
void ValueT<short>::get(unsigned char &v) {
    NOTIMP;
}

template<>
void ValueT<short>::get(long &v) {
    NOTIMP;
}

template<>
void ValueT<short>::get(float &v) {
    NOTIMP;
}

template<>
void ValueT<short>::get(std::string &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<long>::get(double &v) {
    NOTIMP;
}

template<>
void ValueT<long>::get(short &v) {
    NOTIMP;
}

template<>
void ValueT<long>::get(long &v) {
    v = value_;
}

template<>
void ValueT<long>::get(unsigned char &v) {
    NOTIMP;
}

template<>
void ValueT<long>::get(float &v) {
    NOTIMP;
}

template<>
void ValueT<long>::get(std::string &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<float>::get(double &v) {
    NOTIMP;
}

template<>
void ValueT<float>::get(short &v) {
    NOTIMP;
}

template<>
void ValueT<float>::get(unsigned char &v) {
    NOTIMP;
}

template<>
void ValueT<float>::get(long &v) {
    NOTIMP;
}

template<>
void ValueT<float>::get(float &v) {
    v = value_;
}

template<>
void ValueT<float>::get(std::string &v) {
    NOTIMP;
}

//=============================
template<>
void ValueT<double>::get(double &v) {
    v = value_;
}

template<>
void ValueT<double>::get(short &v) {
    NOTIMP;
}

template<>
void ValueT<double>::get(unsigned char &v) {
    NOTIMP;
}

template<>
void ValueT<double>::get(long &v) {
    NOTIMP;
}

template<>
void ValueT<double>::get(float &v) {
    NOTIMP;
}

template<>
void ValueT<double>::get(std::string &v) {
    NOTIMP;
}
//=============================
template<>
void ValueT<std::string>::get(double &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::get(short &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::get(unsigned char &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::get(long &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::get(float &v) {
    NOTIMP;
}
template<>
void ValueT<std::string>::get(std::string &v) {
    NOTIMP;
}

