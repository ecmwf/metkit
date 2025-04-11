/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_FieldIndexList_H
#define metkit_FieldIndexList_H

#include "eckit/io/Offset.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/types/Types.h"

namespace metkit {
namespace fields {

class FieldIndex;

class FieldIndexList {
public:

    ~FieldIndexList();

    void readFrom(eckit::Stream& s);
    void sendTo(eckit::Stream& s) const;

    eckit::OffsetList offset_;
    eckit::LengthList length_;
    std::vector<FieldIndex*> fields_;
};

}  // namespace fields
}  // namespace metkit


#endif
