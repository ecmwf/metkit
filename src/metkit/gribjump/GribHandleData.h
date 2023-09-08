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

// xxx pending better name...
class JumpHandle : public eckit::NonCopyable {
public:

    JumpHandle(const eckit::PathName&);

    /// constructor taking ownership of a handle pointer
    JumpHandle(eckit::DataHandle*);

    ~JumpHandle();

    const JumpInfo& extractFileToBin(eckit::PathName&);
    const JumpInfo& extractNext();
    eckit::Offset handlePosition();
    eckit::Length handleSize();
    eckit::Offset seek(const eckit::Offset&) const;

private:

    mutable eckit::DataHandle *handle_;
    eckit::PathName path_;
    bool ownsHandle_;
    mutable bool opened_;

    mutable pointdb::GribFieldInfo infoOld_;
    mutable JumpInfo info_;

    virtual long read(void*, long) const;
    virtual void print(std::ostream& s) const;

    void open() const;
    void close() const;

    friend class JumpInfo;
};

} // namespace gribjump
} // namespace metkit

#endif
