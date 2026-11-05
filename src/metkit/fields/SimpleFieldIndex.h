// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "metkit/fields/FieldIndex.h"
namespace metkit::fields {
// struct grib_handle;

class SimpleFieldIndex : public FieldIndex {
public:

    // -- Contructors

    SimpleFieldIndex(eckit::Stream&);

    // -- Destructor

    virtual ~SimpleFieldIndex() override;

    // -- Methods

private:  // members

    // friend std::ostream& operator<<(std::ostream& s,const SimpleFieldIndex& p)
    //	{ p.print(s); return s; }
};

}  // namespace metkit::fields
