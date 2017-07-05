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

#ifndef metkit_netcdf_Mapper
#define metkit_netcdf_Mapper

#include "metkit/netcdf/Exceptions.h"
#include "metkit/netcdf/Reshape.h"

#include <iostream>

namespace metkit{
namespace netcdf{

template<class T>

class Mapper {
public:
    Mapper(std::vector<T> &v, std::vector<bool> &set, bool &overlap);
    Mapper(Mapper<T> &parent, const std::vector<Reshape *> &reshape);
    ~Mapper();

    void overlap(bool on) {
        overlap_ = on;
    }

    void set(size_t i, T value);


private:

    std::vector<T> &v_;
    std::vector<bool> &set_;
    std::vector<Reshape *> reshapes_;
    bool &overlap_;

};

template<class T>
Mapper<T>::Mapper(std::vector<T> &v, std::vector<bool> &set, bool &overlap):
    v_(v), set_(set), overlap_(overlap)
{
}

template<class T>
Mapper<T>::Mapper(Mapper<T> &parent, const std::vector<Reshape *> &reshapes):
    v_(parent.v_),
    set_(parent.set_),
    overlap_(parent.overlap_)
{
    Reshape *r = 0;
    for (std::vector<Reshape *>::const_iterator j = reshapes.begin(); j != reshapes.end(); ++j) {
        if (r && r->merge(*(*j))) {
            // Pass
        }
        else {
            r = (*j);
            r->attach();
            reshapes_.push_back(r);
        }
    }

    for (std::vector<Reshape *>::iterator j = parent.reshapes_.begin(); j != parent.reshapes_.end(); ++j) {
        if (r && r->merge(*(*j))) {
            // Pass
        }
        else {
            r = (*j);
            r->attach();
            reshapes_.push_back(r);
        }
    }
}

template<class T>
Mapper<T>::~Mapper() {
    for (std::vector<Reshape *>::iterator j = reshapes_.begin(); j != reshapes_.end(); ++j) {
        (*j)->detach();
    }
}

template<class T>
void Mapper<T>::set(size_t i, T v) {

    for (std::vector<Reshape *>::iterator j = reshapes_.begin(); j != reshapes_.end(); ++j) {
        Reshape *m = (*j);
        size_t k = (*m)(i);
        ASSERT(k >= i);
        i = k;
    }

    ASSERT(i < v_.size());

    if (!overlap_)
    {
        if (v_[i] != v)
        {
            ASSERT(!set_[i]);
        }
    }
    set_[i] = true;

    v_[i] = v;
}

}
}
#endif
