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

#ifndef metkit_netcdf_NCFile
#define metkit_netcdf_NCFile

#include <string>

namespace metkit{
namespace netcdf{

class NCFile
{
public:
    NCFile(const std::string &path);
    ~NCFile();

    int open();
    void close();

    const std::string &path() const;

protected:
    // -- Members
    std::string path_;
    int nc_;
    bool open_;

private:

    NCFile(const NCFile &);
    NCFile &operator=(const NCFile &);

    // -- Methods
    void print(std::ostream &out) const;


    // -- Friends
    friend std::ostream &operator<<(std::ostream &out, const NCFile &v)
    {
        v.print(out);
        return out;
    }
};


}
}
#endif
