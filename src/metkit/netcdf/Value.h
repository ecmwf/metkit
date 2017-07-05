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

#ifndef Value_H
#define Value_H

#include <string>
#include <vector>

class Type;

class Value
{
public:
    Value(Type &type): type_(type) {}
    virtual ~Value() {}

    virtual void createAttribute(int nc, int varid, const std::string &name, const std::string &path) = 0;
    virtual Value *clone() const = 0;
    virtual void dump(std::ostream &out) const = 0;

    virtual bool sameAs(const Value &other) const = 0;
    virtual std::string asString() const = 0;

    virtual void fill(const std::vector<bool> &set, std::vector<double> &v) = 0;
    virtual void fill(const std::vector<bool> &set, std::vector<short> &v) = 0;
    virtual void fill(const std::vector<bool> &set, std::vector<unsigned char> &v) = 0;
    virtual void fill(const std::vector<bool> &set, std::vector<long> &v) = 0;
    virtual void fill(const std::vector<bool> &set, std::vector<float> &v) = 0;
    virtual void fill(const std::vector<bool> &set, std::vector<std::string> &v) = 0;

    virtual void clear(std::vector<bool> &set, const std::vector<double> &v) = 0;
    virtual void clear(std::vector<bool> &set, const std::vector<short> &v) = 0;
    virtual void clear(std::vector<bool> &set, const std::vector<unsigned char> &v) = 0;
    virtual void clear(std::vector<bool> &set, const std::vector<long> &v) = 0;
    virtual void clear(std::vector<bool> &set, const std::vector<float> &v) = 0;
    virtual void clear(std::vector<bool> &set, const std::vector<std::string> &v) = 0;

    virtual void init(std::vector<double> &v, size_t size) = 0;
    virtual void init(std::vector<short> &v, size_t size) = 0;
    virtual void init(std::vector<unsigned char> &v, size_t size) = 0;
    virtual void init(std::vector<long> &v, size_t size) = 0;
    virtual void init(std::vector<float> &v, size_t size) = 0;
    virtual void init(std::vector<std::string> &v, size_t size) = 0;

    virtual void get(double &v) = 0;
    virtual void get(short &v) = 0;
    virtual void get(unsigned char &v) = 0;
    virtual void get(long &v) = 0;
    virtual void get(float &v) = 0;
    virtual void get(std::string &v) = 0;

    static Value *newFromString(const std::string &);

protected:
    // -- Members
    Type &type_;

private:

    Value(const Value &);
    Value &operator=(const Value &);

    // -- Methods
    virtual void print(std::ostream &out) const = 0;


    // -- Friends
    friend std::ostream &operator<<(std::ostream &out, const Value &v)
    {
        v.print(out);
        return out;
    }
};

template<class T>
class ValueT : public Value
{
public:
    ValueT(Type &type, const T &value) : Value(type), value_(value) {}
    virtual void createAttribute(int nc, int varid, const std::string &name, const std::string &path);


private:
    T value_;

    virtual void print(std::ostream &out) const
    {
        out << value_;
    }

    virtual void dump(std::ostream &out) const;
    virtual std::string asString() const;

    virtual Value *clone() const {
        return new ValueT(type_, value_);
    }

    virtual bool sameAs(const Value &other) const {
        const ValueT *o = dynamic_cast<const ValueT *>(&other);
        if (o) {
            return o->value_ == value_;
        }
        return false;
    }

    virtual void fill(const std::vector<bool> &set, std::vector<double> &v);
    virtual void fill(const std::vector<bool> &set, std::vector<short> &v);
    virtual void fill(const std::vector<bool> &set, std::vector<unsigned char> &v);
    virtual void fill(const std::vector<bool> &set, std::vector<long> &v);
    virtual void fill(const std::vector<bool> &set, std::vector<float> &v);
    virtual void fill(const std::vector<bool> &set, std::vector<std::string> &v);

    virtual void clear(std::vector<bool> &set, const std::vector<double> &v);
    virtual void clear(std::vector<bool> &set, const std::vector<short> &v);
    virtual void clear(std::vector<bool> &set, const std::vector<unsigned char> &v);
    virtual void clear(std::vector<bool> &set, const std::vector<long> &v);
    virtual void clear(std::vector<bool> &set, const std::vector<float> &v);
    virtual void clear(std::vector<bool> &set, const std::vector<std::string> &v);

    virtual void init(std::vector<double> &v, size_t size);
    virtual void init(std::vector<short> &v, size_t size);
    virtual void init(std::vector<unsigned char> &v, size_t size);
    virtual void init(std::vector<long> &v, size_t size);
    virtual void init(std::vector<float> &v, size_t size);
    virtual void init(std::vector<std::string> &v, size_t size);

    virtual void get(double &v);
    virtual void get(short &v);
    virtual void get(unsigned char &v);
    virtual void get(long &v);
    virtual void get(float &v);
    virtual void get(std::string &v);

};

#endif
