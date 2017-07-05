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

#ifndef metkit_netcdf_Attribute
#define metkit_netcdf_Attribute

#include <iosfwd>
#include <string>

namespace metkit {
namespace netcdf {

class Endowed;
class Value;

class Attribute {
public:

    Attribute(Endowed &owner, const std::string &name, Value *value);
    virtual ~Attribute();

    // -- Methods
    virtual bool sameAs(const Attribute &) const;
    virtual void dump(std::ostream &out) const;
    virtual void create(int nc) const;
    virtual void clone(Endowed &owner) const;
    virtual void merge(const Attribute &);
    virtual void invalidate();

    virtual std::string asString() const;

    const std::string &name() const;
    std::string fullName() const;

    const Value &value() const;

protected:
    // -- Members

    Endowed &owner_;
    std::string name_;
    Value *value_;

private:

    Attribute(const Attribute &);
    Attribute &operator=(const Attribute &);

    // -- Methods

    virtual void print(std::ostream &out) const = 0;

    // Friends
    friend std::ostream &operator<<(std::ostream &out, const Attribute &v)
    {
        v.print(out);
        return out;
    }

};

}
}
#endif
