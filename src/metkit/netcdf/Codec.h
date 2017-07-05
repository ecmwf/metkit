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

#ifndef metkit_netcdf_Codec
#define metkit_netcdf_Codec

#include <iosfwd>
#include <vector>

#include "metkit/netcdf/Counted.h"

namespace metkit{
namespace netcdf{

class Variable;

class Codec : public Counted
{
public:
    Codec();
    virtual ~Codec();

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

protected:
    // -- Members

private:

    Codec(const Codec &);
    Codec &operator=(const Codec &);

    // -- Methods
    virtual void print(std::ostream &out) const = 0;


    // -- Friends
    friend std::ostream &operator<<(std::ostream &out, const Codec &v)
    {
        v.print(out);
        return out;
    }
};


}
}

#endif
