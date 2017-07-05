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

#ifndef metkit_netcdf_Matrix
#define metkit_netcdf_Matrix



#include "Counted.h"

#include <string>
#include <vector>

namespace metkit{
namespace netcdf{

class Type;
class Value;
class Remapping;
class Codec;

class Reshape;
template<class T> class Mapper;

class Matrix : public Counted
{
public:

    Matrix(Type &type, const std::string &name, size_t size);


    Type &type() const;
    void type(Type &);

    const std::string &name() const;
    size_t size() const;

    Matrix *mergeData(Matrix *other, size_t size);
    Matrix *mergeCoordinates(Matrix *other, size_t size);

    template<class T>
    const std::vector<T> &values() const;

public:

    void decache() const;

    void dump(std::ostream &) const;
    virtual void dumpTree(std::ostream &, size_t) const;
    void save(int nc, int varid, const std::string &path) const;

    void printValues(std::ostream &) const;
    void missingValue(Value *) ;
    Value *missingValue() const;

    void codec(Codec *);
    Codec *codec() const;

    virtual Matrix *merged();
    // ========================================================

    void reshape(Reshape *);
    const std::vector<Reshape *> &reshape() const;

    // ========================================================

    virtual void fill(Mapper<double> &) const;
    virtual void fill(Mapper<float> &) const;
    virtual void fill(Mapper<short> &) const;
    virtual void fill(Mapper<long> &) const;
    virtual void fill(Mapper<unsigned char> &) const;

protected:

    virtual ~Matrix();

    // -- Members

    Type *type_;
    Value *missingValue_;
    Codec *codec_;
    std::string name_;
    size_t size_;
    std::vector<Reshape *> reshapes_;

private:

    Matrix(const Matrix &);
    Matrix &operator=(const Matrix &);

    // -- Members
    mutable std::vector<double> double_cache_;
    mutable std::vector<float> float_cache_;
    mutable std::vector<long> long_cache_;
    mutable std::vector<short> short_cache_;
    mutable std::vector<unsigned char> unsigned_char_cache_;
    mutable std::vector<std::string> string_cache_;

    void values(std::vector<double> &) const;
    void values(std::vector<float> &) const;
    void values(std::vector<long> &) const;
    void values(std::vector<short> &) const;
    void values(std::vector<unsigned char> &) const;
    void values(std::vector<std::string> &) const;


    // -- Methods
    template<class T> void _fill(std::vector<T> &v) const;

    template<class T>
    const std::vector<T> &cache() const;

    virtual void print(std::ostream &out) const = 0;


    // -- Friends
    friend std::ostream &operator<<(std::ostream &out, const Matrix &v)
    {
        v.print(out);
        return out;
    }
};


template<class T>
const std::vector<T> &Matrix::values() const {
    const std::vector<T> &vals = cache<T>();
    if (vals.empty()) {
        values(const_cast<std::vector<T>&>(vals));
    }
    return vals;
}

}
}
#endif
