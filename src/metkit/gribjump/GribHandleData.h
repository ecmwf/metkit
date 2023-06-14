/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_GribHandleDataSource2_H
#define metkit_GribHandleDataSource2_H

#include "metkit/pointdb/GribDataSource.h"
#include "metkit/gribjump/GribInfo.h"
#include "metkit/pointdb/GribFieldInfo.h"

namespace eckit {
class DataHandle;
}

namespace metkit {
namespace gribjump {


class GribHandleData : public eckit::NonCopyable {
public:

    GribHandleData(const eckit::PathName&);
    ~GribHandleData();

    const GribInfo& extractMetadata(eckit::PathName&);

private:

    mutable eckit::DataHandle *handle_;
    eckit::PathName gribpath_;
    bool ownsHandle_;
    mutable bool opened_;

    mutable pointdb::GribFieldInfo infoOld_;
    mutable GribInfo info_;

    virtual eckit::Offset seek(const eckit::Offset&) const;
    virtual long read(void*, long) const;
    virtual void print(std::ostream& s) const;

    void open() const;
    void close() const;

    friend class GribInfo;
};

} // namespace gribjump
} // namespace metkit

#endif
