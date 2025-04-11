/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_SimpleFieldIndex_H
#define metkit_SimpleFieldIndex_H

#include "metkit/fields/FieldIndex.h"
namespace metkit {
namespace fields {
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

}  // namespace fields
}  // namespace metkit

#endif
