/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File HyperCube.h
// Baudouin Raoult - ECMWF Oct 96

#ifndef metkit_netcdf_eckityperCube_h
#define metkit_netcdf_eckityperCube_h

#include <vector>
#include <functional>
#include <numeric>
#include <ostream>

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

// Helper class to handle multi-dimension objects
// The first dimension should be the one most likely to change

namespace metkit{
namespace netcdf{

class HyperCube {
public:


    typedef std::vector<size_t> Dimensions;
    typedef std::vector<size_t> Coordinates;
    typedef std::vector<size_t> Remapping;

    // -- Contructors

    HyperCube(const Dimensions &d) : dimensions_(d) {}

    // -- Methods

    // Translate coordinates into an index to a 1 dimension array.

    size_t index(const Coordinates &) const;

    // Return the number of elemets

    size_t count() const;

    // Translate index to coordinates

    void coordinates(size_t index, Coordinates &) const;

    // Accessors

    const Dimensions &dimensions() const    {
        return dimensions_;
    }
    Dimensions &dimensions()                {
        return dimensions_;
    }
    size_t     dimensions(size_t n) const {
        return dimensions_[n];
    }
    size_t     size() const                {
        return dimensions_.size();
    }

    // Return the 'remapping' std::vector needing to add 'count' labels
    // for the dimension 'which' at position 'where'

    HyperCube addToDimension(size_t which, size_t where, size_t count, Remapping &) const;

    // Combine two 'remapping' vectors

    static void combine(Remapping &,  Remapping &);

private:

    // -- Members

    Dimensions dimensions_;

};

inline  // For speed
size_t HyperCube::count() const
{
    return std::accumulate(dimensions_.begin(), dimensions_.end(), 1, std::multiplies<size_t>());
}


inline // For speed
size_t HyperCube::index(const Coordinates &coord) const
{
    size_t n = 1;
    size_t a = 0;

    // ASSERT(coord.size() == dimensions_.size());

    // The fact that this is in reverse is important for addToDimension

    for (int i = coord.size() - 1; i >= 0; i--)
    {
        // ASSERT(coord[i] >= 0 && coord[i] < dimensions_[i]);
        a += coord[i] * n;
        n *= dimensions_[i];
    }

    return a;
}


inline // For speed
void HyperCube::combine(Remapping &map1, Remapping &map2)
{
    if (map1.size() == 0)
    {
        std::swap(map1, map2);
    }
    else {
        for (Remapping::iterator i = map1.begin() ; i != map1.end() ; ++i)
        {
            // ASSERT(*i < map2.size());
            *i = map2[*i];
        }
    }
}

std::ostream &operator<<(std::ostream &out, const HyperCube &cube);

//-----------------------------------------------------------------------------


}
}
#endif
