// SPDX-FileCopyrightText: 1996-2013 European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "metkit/pointdb/GribDataSource.h"
#include "metkit/pointdb/GribFieldInfo.h"

namespace eckit {
class DataHandle;
}

namespace metkit::pointdb {


class GribHandleDataSource : public GribDataSource {
public:

    GribHandleDataSource(const eckit::PathName&, const eckit::Offset& = 0);
    GribHandleDataSource(eckit::DataHandle&, const eckit::Offset& = 0);
    GribHandleDataSource(eckit::DataHandle*, const eckit::Offset& = 0);

    ~GribHandleDataSource() override;

private:

    mutable eckit::DataHandle* handle_;
    bool ownsHandle_;
    mutable bool opened_;

    mutable GribFieldInfo info_;
    eckit::Offset offset_;

    virtual eckit::Offset seek(const eckit::Offset&) const override;
    virtual long read(void*, long) const override;
    virtual const GribFieldInfo& info() const override;
    virtual void print(std::ostream& s) const override;
    virtual const std::map<std::string, eckit::Value>& request() const override;
    virtual std::string groupKey() const override;
    virtual std::string sortKey() const override;

    void open() const;
};

}  // namespace metkit::pointdb
