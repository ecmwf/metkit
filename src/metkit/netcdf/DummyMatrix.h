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

#ifndef metkit_netcdf_DummyMatrix
#define metkit_netcdf_DummyMatrix

#include "metkit/netcdf/Matrix.h"

namespace metkit{
namespace netcdf{

class Variable;


class DummyMatrix : public Matrix {
public:
    DummyMatrix(const Variable &);
    virtual ~DummyMatrix();

private:

    Matrix *other_;
    bool merged_;

    // Methods

    virtual void print(std::ostream &out) const;
    virtual void fill(Mapper<double> &) const;
    virtual void fill(Mapper<float> &) const;
    virtual void fill(Mapper<long> &) const;
    virtual void fill(Mapper<short> &) const;
    virtual void fill(Mapper<unsigned char> &) const;


    virtual Matrix *merged();

};

}
}
#endif
