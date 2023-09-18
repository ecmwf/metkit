/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_JumpHandle_H
#define metkit_JumpHandle_H

#include "eckit/filesystem/PathName.h"
#include "metkit/gribjump/GribInfo.h"

namespace eckit {
class DataHandle;
}

namespace metkit {
namespace gribjump {

class JumpHandle : public eckit::NonCopyable {
public:

    JumpHandle(const eckit::PathName&);

    /// Takes ownership of a handle pointer
    JumpHandle(eckit::DataHandle*);

    ~JumpHandle();

    const JumpInfo& extractInfoFromFile(eckit::PathName&);
    const JumpInfo& extractInfo();
    eckit::Offset position();
    eckit::Length size();
    eckit::Offset seek(const eckit::Offset&) const;

private:

    mutable eckit::DataHandle *handle_;
    bool ownsHandle_;
    mutable bool opened_;
    eckit::PathName path_;

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
