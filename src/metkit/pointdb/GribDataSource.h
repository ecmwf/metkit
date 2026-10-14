// SPDX-FileCopyrightText: 1996-2013 European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "metkit/pointdb/DataSource.h"
#include "metkit/pointdb/PointIndex.h"

namespace eckit {
class Offset;
}

namespace metkit::pointdb {

class GribFieldInfo;

class GribDataSource : public DataSource {
public:

    ~GribDataSource() override = default;
    PointResult extract(double lat, double lon) const override;

private:

    virtual double value(size_t index) const;
    virtual std::string geographyHash() const;


    virtual eckit::Offset seek(const eckit::Offset&) const = 0;
    virtual long read(void*, long) const                   = 0;
    virtual const GribFieldInfo& info() const              = 0;

    friend class GribFieldInfo;
};

}  // namespace metkit::pointdb
