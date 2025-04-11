/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_GribDataSource_H
#define metkit_GribDataSource_H

#include "metkit/pointdb/DataSource.h"
#include "metkit/pointdb/PointIndex.h"

namespace eckit {
class Offset;
}

namespace metkit {
namespace pointdb {

class GribFieldInfo;

class GribDataSource : public DataSource {
public:

    virtual PointResult extract(double lat, double lon) const;

private:

    virtual double value(size_t index) const;
    virtual std::string geographyHash() const;


    virtual eckit::Offset seek(const eckit::Offset&) const = 0;
    virtual long read(void*, long) const                   = 0;
    virtual const GribFieldInfo& info() const              = 0;

    friend class GribFieldInfo;
};

}  // namespace pointdb
}  // namespace metkit

#endif
