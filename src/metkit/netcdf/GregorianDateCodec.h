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

#ifndef GregorianDateCodec_H
#define GregorianDateCodec_H

#include "Codec.h"
#include "DateTime.h"

class Variable;

class GregorianDateCodec : public Codec
{
public:

    GregorianDateCodec(const std::string &unit, const std::string &calendar);
    virtual ~GregorianDateCodec();

private:

    DateTime reference_;
    long long offset_;
    mutable long long zero_;

    std::string units_;
    std::string calendar_;

    // -- Methods

    virtual void print(std::ostream &s) const;

    template<class T>
    void _decode(std::vector<T> &v) const;

    virtual void decode(std::vector<double> &) const ;
    virtual void decode(std::vector<float> &) const ;
    virtual void decode(std::vector<long> &) const ;
    virtual void decode(std::vector<short> &) const ;
    virtual void decode(std::vector<unsigned char> &) const ;

    virtual void encode(std::vector<double> &) const ;
    virtual void encode(std::vector<float> &) const ;
    virtual void encode(std::vector<long> &) const ;
    virtual void encode(std::vector<short> &) const ;
    virtual void encode(std::vector<unsigned char> &) const ;

    virtual void addAttributes(Variable &) const;
    virtual void updateAttributes(int nc, int varid, const std::string &path);

    virtual bool timeAxis() const;

};

#endif
