/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_SimpleGribDataSource_H
#define metkit_SimpleGribDataSource_H

#include "metkit/pointdb/GribDataSource.h"
#include "metkit/pointdb/GribFieldInfo.h"

namespace eckit {
class DataHandle;
}

namespace metkit {
namespace pointdb {


class SimpleGribDataSource : public GribDataSource {
public:

    SimpleGribDataSource(const eckit::PathName&, const eckit::Offset& = 0);
    SimpleGribDataSource( eckit::DataHandle&, const eckit::Offset& = 0);
    SimpleGribDataSource( eckit::DataHandle*, const eckit::Offset& = 0);


    ~SimpleGribDataSource();

private:

    mutable eckit::DataHandle *handle_;
    bool ownsHandle_;
    mutable bool opened_;

    mutable GribFieldInfo info_;
    eckit::Offset offset_;

    virtual eckit::Offset seek(const eckit::Offset&) const;
    virtual long read(void*, long) const;
    virtual const GribFieldInfo& info() const;
    virtual void print(std::ostream& s) const;
    virtual void request(eckit::JSON&) const;

    void open() const;

};

} // namespace pointdb
} // namespace metkit

#endif
