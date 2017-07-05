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

#ifndef metkit_netcdf_Dimension
#define metkit_netcdf_Dimension

#include <iosfwd>
#include <string>

namespace metkit{
namespace netcdf{

class Field;
class Remapping;


class Dimension {
public:
    Dimension(Field &owner, const std::string &name, size_t len);
    virtual ~Dimension();

    // -- Methods
    virtual int id() const;

    const std::string &name() const;
    size_t count() const;

    virtual bool sameAs(const Dimension &) const;
    virtual void dump(std::ostream &s) const;
    virtual void create(int nc) const;
    virtual void clone(Field &owner) const;
    virtual void grow(size_t n);
    virtual bool inUse() const;

    virtual void remapping(Remapping *) ;
    virtual const Remapping &remapping() const;


protected:

    // -- Members

    Field &owner_;
    std::string name_;
    size_t len_;
    Remapping *remapping_;


private:

    // -- Methods
    virtual void print(std::ostream &s) const = 0;

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Dimension &v) {
        v.print(s);
        return s;
    }

};

}
}
#endif
