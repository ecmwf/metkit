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

#ifndef metkit_netcdf_Field
#define metkit_netcdf_Field

#include "Endowed.h"
#include <string>
#include <vector>

namespace metkit{
namespace netcdf{

class Dimension;
class Variable;


class Field : public Endowed {
public:

    Field(const std::string &);
    virtual ~Field();

    // -- Methods

    Dimension *findDimension(int id) const;
    Dimension *findDimension(const std::string &name) const;
    std::vector<Variable *> variablesForDimension(const Dimension &) const;

    virtual void dump(std::ostream &s) const;

    void add(Dimension *);
    void add(Variable *);

    const std::map<std::string, Dimension *> &dimensions() const ;
    const std::map<std::string, Variable *> &variables() const ;

    // From Endowed

    virtual const std::string &path() const;

protected:

    // -- Members
    std::string path_;
    std::map<std::string, Dimension *> dimensions_;
    std::map<std::string, Variable *> variables_;


private:

    Field(const Field &);
    Field &operator=(const Field &);

    // From Endowed

    virtual int varid() const;
    virtual const std::string &name() const;

    // - Methods

    virtual void print(std::ostream &s) const = 0;

    // -- Friends
    friend std::ostream &operator<<(std::ostream &s, const Field &v) {
        v.print(s);
        return s;
    }
};

}
}
#endif
