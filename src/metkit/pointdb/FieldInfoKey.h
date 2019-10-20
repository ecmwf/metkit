/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef FieldInfoKey_H
#define FieldInfoKey_H

// #include <iosfwd>
#include <map>

// #include "eckit/eckit.h"

// #include "pointdb/FieldInfoKey.h"

class IndexFile;
namespace eckit { class JSON; }
namespace eckit { class Value; }
namespace metkit {
    namespace grib {
        class GribHandle;
    }
}
struct grib_handle;


class FieldInfoKey {
public:
    FieldInfoKey();

    bool operator<(const FieldInfoKey& other) const;
    bool operator==(const FieldInfoKey& other) const;
    void print(std::ostream& s) const;
    bool match(const FieldInfoKey& k1, const FieldInfoKey& k2) const;
    void update(const metkit::grib::GribHandle&);

    void json(eckit::JSON&) const;

    static FieldInfoKey minimum();
    static FieldInfoKey maximum();

    void fill(const eckit::Value&);
    void fill(const std::map<std::string,std::string>&);

    void param(unsigned long p) { paramId_ = p; }

private:

    char  type_;
    char            levtype_;
    unsigned long   paramId_;
    long            endStep_;
    unsigned long   level_;
    unsigned long   number_;



    friend std::ostream& operator<<(std::ostream& s, const FieldInfoKey& f)
        { f.print(s); return s; }

    // TODO: get rid of that
    friend class GribFileSummary;
};


#endif
