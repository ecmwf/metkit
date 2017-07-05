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

#include "Type.h"

#include "Codec.h"
#include "Dimension.h"
#include "Exceptions.h"
#include "Matrix.h"
#include "MergePlan.h"
#include "Remapping.h"
#include "UpdateCoordinateStep.h"
#include "Value.h"
#include "Variable.h"

#include <iostream>
#include <vector>
#include <algorithm>

#include <netcdf.h>

namespace metkit{
namespace netcdf{
static Type *types_[NC_MAX_ATOMIC_TYPE + 1] = {0,};

Type::Type(int code, const std::string &name, const std::string &dump, int super):
    code_(code),
    super_(super),
    name_(name),
    dump_(dump)
{
    types_[code] = this;
}

Type &Type::lookup(int type)
{
    ASSERT(type >= 0 && type <= NC_MAX_ATOMIC_TYPE);
    ASSERT(types_[type] != 0);
    return *types_[type];
}

Type &Type::lookup(Type &type1, Type &type2)
{
    if (type1 == type2) {
        return type1;
    }


    Type *t1 = &type1;
    Type *t2 = &type2;

    std::vector<int> s1;
    std::vector<int> s2;

    while (t1->code_ != -1) {
        s1.push_back(t1->code_);
        if (t1->super_ < 0) break;
        t1 = &lookup(t1->super_);
    }

    while (t2->code_ != -1) {
        s2.push_back(t2->code_);
        if (t2->super_ < 0) break;
        t2 = &lookup(t2->super_);
    }

    for (std::vector<int>::const_iterator j = s1.begin(); j != s1.end(); ++j) {
        std::vector<int>::const_iterator k = std::find(s2.begin(), s2.end(), *j);
        if (k != s2.end()) {

            if (type1 != type2) {
                std::cout << "Common super-type for " << type1 << " and " << type2 << " is " << lookup(*j) << std::endl;
            }

            return lookup(*j);
        }

    }

    std::stringstream s;
    s << "Cannot find a common super-type to " << type1 << " and " << type2;
    throw MergeError(s.str());

}

void Type::dump(std::ostream &out) const {
    out << dump_;
}

bool Type::operator==(const Type &other) const {
    return code_ == other.code_;
}

bool Type::operator!=(const Type &other) const {
    return code_ != other.code_;
}

//===============================================================================================

template<class T>
class TypeT : public Type
{

public:
    TypeT(int code, const std::string &name, const std::string &dump, int super) : Type(code, name, dump, super) {}

private:
    virtual Value *attributeValue(int nc, int id, const char *name, size_t len, const std::string &path);

    virtual bool coordinateOutputVariableMerge(Variable &, const Variable &other, MergePlan &plan);
    virtual bool cellMethodOutputVariableMerge(Variable &a, const Variable &b, MergePlan &plan);
    virtual void save(const Matrix &, int nc, int varid, const std::string &path) const;

    virtual void print(std::ostream &out) const;
    virtual void dump(std::ostream &out, const Matrix &) const;
    virtual void printValues(std::ostream &out, const Matrix &) const;

};

template<class T>
void TypeT<T>::print(std::ostream &out) const
{
    out << name_;
}

template<class T>
void TypeT<T>::dump(std::ostream &out, const Matrix &matrix) const
{
    const typename std::vector<T> &v = matrix.values<T>();
    size_t i = 0;
    for (typename std::vector<T>::const_iterator k = v.begin(); k != v.end(); ++k) {
        out << ' ' << *k;
        i++;
        if (i < v.size()) {
            out << ',';
        }

        if ((i % 7) == 0) {
            out << std::endl;
        }
    }
}

template<class T>
void TypeT<T>::printValues(std::ostream &out, const Matrix &matrix) const
{
    const typename  std::vector<T> v = matrix.values<T>();
    size_t i = 0;
    for (typename std::vector<T>::const_iterator k = v.begin(); k != v.end(); ++k) {
        out << *k;
        i++;
        if (i < v.size()) {
            out << ',';
        }
    }
}

//=======================================================================================================
template<>
bool TypeT<std::string>::cellMethodOutputVariableMerge( Variable &out, const Variable &in, MergePlan &plan)
{
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return false;
}

template<class T>
bool TypeT<T>::cellMethodOutputVariableMerge( Variable &out, const Variable &in, MergePlan &plan) {
    const std::vector<T> &a = out.matrix()->values<T>();
    const std::vector<T> &b = in.matrix()->values<T>();

    if (a != b) {
        return true;
    }
    else {
        return false;
    }
}

//=======================================================================================================

template<class T, class Q>
static void save_values(const Matrix &matrix, int nc, int varid, const std::string &path, Q put) {

    // std::cout << "Save " << matrix << std::endl;
    // matrix.dumpTree(std::cout, 0);
    Codec *codec = matrix.codec();
    if (codec) {
        std::vector<T> values = matrix.values<T>();
        codec->encode(values);
        ASSERT(varid >= 0);
        ASSERT(values.size());
        NC_CALL(put(nc, varid, &values[0]), path);
    }
    else
    {
        const std::vector<T> &values = matrix.values<T>();
        ASSERT(varid >= 0);
        ASSERT(values.size());
        NC_CALL(put(nc, varid, &values[0]), path);
    }

    // No need for the values anymore
    matrix.decache();
}

template<>
void TypeT<std::string>::save(const Matrix &, int nc, int varid, const std::string &path) const
{
    std::cout << __FUNCTION__ << " " << *this << std::endl; NOTIMP;
}

template<>
void TypeT<double>::save(const Matrix &m, int out, int varid, const std::string &path)  const {
    save_values<double>(m, out, varid, path, &nc_put_var_double);

}

template<>
void TypeT<float>::save(const Matrix &m, int out, int varid, const std::string &path)  const {
    save_values<float>(m, out, varid, path, &nc_put_var_float);
}

template<>
void TypeT<unsigned char>::save(const Matrix &m, int out, int varid, const std::string &path)  const {
    save_values<unsigned char>(m, out, varid, path, &nc_put_var_ubyte);
}

template<>
void TypeT<long>::save(const Matrix &m, int out, int varid, const std::string &path)  const {
    save_values<long>(m, out, varid, path,  &nc_put_var_long);
}

template<>
void TypeT<short>::save(const Matrix &m, int out, int varid, const std::string &path)  const
{
    save_values<short>(m, out, varid, path,  &nc_put_var_short);
}

//=======================================================================================================
template<>
bool TypeT<std::string>::coordinateOutputVariableMerge( Variable &out, const Variable &in, MergePlan &plan)
{
    std::cout << __FUNCTION__ << " " << *this << std::endl;
    NOTIMP;
    return false;
}

template<class T>
bool TypeT<T>::coordinateOutputVariableMerge( Variable &out, const Variable &in, MergePlan &plan) {

    const std::vector<T> &a = out.matrix()->values<T>();
    const std::vector<T> &b = in.matrix()->values<T>();

    if (a != b) {

        const std::vector<Dimension *> &dimensions = out.dimensions();
        ASSERT(dimensions.size() == 1);

        size_t growth = 0;
        std::vector<size_t> ignore;
        Remapping *remapping = new Remapping();

        for (size_t i = 0; i < b.size(); i++) {
            bool found = false;
            for (size_t j  = 0; j < a.size(); j++) {
                if (b[i] == a[j]) {
                    remapping->push_back(j);
                    ASSERT(j >= i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                remapping->push_back(a.size() + growth);
                growth++;
            }
        }

        remapping->validate();

        plan.add(new UpdateCoordinateStep(out, in, growth));
        dimensions[0]->remapping(remapping);
        return true;

    }
    else {
        return false;
    }
}

//=======================================================================================================

template<>
Value *TypeT<unsigned char>::attributeValue(int nc, int id, const char *name, size_t len, const std::string &path)
{
    unsigned char value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_ubyte (nc, id, name, &value), path);
    return new ValueT<unsigned char>(*this, value);
}

template<>
Value *TypeT<short>::attributeValue(int nc, int id, const char *name, size_t len, const std::string &path)
{
    short value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_short (nc, id, name, &value), path);
    return new ValueT<short>(*this, value);
}

template<>
Value *TypeT<long>::attributeValue(int nc, int id, const char *name, size_t len, const std::string &path)
{
    long value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_long (nc, id, name, &value), path);
    return new ValueT<long>(*this, value);
}

template<>
Value *TypeT<std::string>::attributeValue(int nc, int id, const char *name, size_t len, const std::string &path)
{
    char value[len + 1];
    memset(value, 0, sizeof(value));
    NC_CALL(nc_get_att_text(nc, id, name, value), path);
    return new ValueT<std::string>(*this, value);
}

template<>
Value *TypeT<double>::attributeValue(int nc, int id, const char *name, size_t len, const std::string &path)
{
    double value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_double(nc, id, name, &value), path);
    return new ValueT<double>(*this, value);
}

template<>
Value *TypeT<float>::attributeValue(int nc, int id, const char *name, size_t len, const std::string &path)
{
    float value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_float(nc, id, name, &value), path);
    return new ValueT<float>(*this, value);
}

//=============================


#define T(a, b, c) static TypeT<a> TYPE_##b(b, #b, #a, c)
T(unsigned char , NC_BYTE, NC_SHORT);
T(short, NC_SHORT, NC_LONG);
T(long, NC_LONG, NC_DOUBLE);
T(std::string, NC_CHAR, -1);
T(float, NC_FLOAT, NC_DOUBLE);
T(double, NC_DOUBLE, -1);
}
}
