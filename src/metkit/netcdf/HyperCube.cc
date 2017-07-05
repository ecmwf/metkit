/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/netcdf/HyperCube.h"
#include "metkit/netcdf/Exceptions.h"

namespace metkit {
namespace netcdf {


static void addLoop2(
    int         d,
    size_t      which,
    size_t      where,
    size_t      count,
    size_t      depth,
    const HyperCube::Dimensions  &dims,
    HyperCube::Remapping   &remap,
    const HyperCube::Dimensions  &mul,
    size_t sum)
{
    int k = 0;
    size_t muld = mul[d];
    size_t dimsd = dims[d];
    for ( size_t i = 0; i < dimsd; i++, k++)
    {
        if (which == d && i == where) {
            k += count;
        }

        size_t s = sum + k * muld;
        if (d == depth)
        {
            remap.push_back(s);
        }
        else {
            addLoop2(d + 1, which, where, count, depth, dims, remap, mul, s);
        }
    }
}

HyperCube HyperCube::addToDimension(size_t which,
                                    size_t where,
                                    size_t howMuch,
                                    Remapping &remap) const
{

    remap.clear();
    remap.reserve(count());

    Dimensions  newdims = dimensions_;
    // Coordinates coord(dimensions_.size());


    newdims[which] += howMuch;

    HyperCube target(newdims);

    Dimensions  mul(dimensions_.size());
    size_t n = 1;
    for (int i = mul.size() - 1; i >= 0; i--)
    {
        mul[i] = n;
        n *= newdims[i];
    }
    addLoop2(0, which, where, howMuch,  dimensions_.size() - 1, dimensions_, remap, mul, 0);

    return target;
}


std::ostream &operator<<(std::ostream &out, const HyperCube &cube) {
    const HyperCube::Dimensions &dimensions = cube.dimensions();
    out << "HyperCube[size=" << cube.count() << ",coordinates=";
    char sep = '(';
    for (HyperCube::Dimensions::const_iterator j = dimensions.begin(); j != dimensions.end(); ++j) {
        out << sep << *j;
        sep = ',';
    }
    out << ')';
    out << ']';
    return out;
}

}
}
