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

#ifndef metkit_netcdf_Endowned
#define metkit_netcdf_Endowned

#include <string>
#include <map>

namespace metkit {
namespace netcdf {

class Attribute;

class Endowed {
public:
    Endowed();
    virtual ~Endowed();

    // -- Methods
    void add(Attribute *);

    void getAttributes(int nc, int i, int nattr);
    void copyAttributes(const Endowed &other);
    void mergeAttributes(const Endowed &other);

    virtual const std::string &path() const = 0;
    virtual int varid() const = 0;
    virtual const std::string &name() const = 0;

    const std::map<std::string, Attribute *> &attributes() const ;

protected:

    // -- Members

    std::map<std::string, Attribute *> attributes_;

private:

    Endowed(const Endowed &);
    Endowed &operator=(const Endowed &);

};

}
}
#endif
