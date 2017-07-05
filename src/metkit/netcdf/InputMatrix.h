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

#ifndef InputMatrix_H
#define InputMatrix_H

#include "Matrix.h"

class NCFile;

class InputMatrix : public Matrix {
public:
    InputMatrix(Type &type, int varid, const std::string &name, size_t size, NCFile &file);
    virtual ~InputMatrix();

private:
    int varid_;
    NCFile &file_;

    // Methods
    void read(std::vector<double> &) const ;
    void read(std::vector<float> &) const ;
    void read(std::vector<long> &) const ;
    void read(std::vector<short> &) const ;
    void read(std::vector<unsigned char> &) const ;

    template<class T> void _fill(Mapper<T> &v) const;

    // Overridden methods
    virtual void fill(Mapper<double> &) const;
    virtual void fill(Mapper<float> &) const;
    virtual void fill(Mapper<long> &) const;
    virtual void fill(Mapper<short> &) const;
    virtual void fill(Mapper<unsigned char> &) const;

    virtual void print(std::ostream &out)  const;


};

#endif
