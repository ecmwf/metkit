/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/exception/Exceptions.h"

#include "metkit/fields/FieldIndexList.h"
#include "metkit/fields/SimpleFieldIndex.h"

namespace metkit {
namespace fields {

FieldIndexList::~FieldIndexList() {
    for (size_t i = 0; i < fields_.size(); ++i) {
        FieldIndex* h = fields_[i];
        delete h;
    }
}

void FieldIndexList::readFrom(eckit::Stream& s) {
    ASSERT(length_.size() == 0);
    ASSERT(offset_.size() == 0);
    ASSERT(fields_.size() == 0);

    unsigned long count;

    s >> count;

    length_.resize(count);
    offset_.resize(count);
    fields_.resize(count);

    for (size_t i = 0; i < count; ++i) {
        unsigned long long x;
        s >> x;
        offset_[i] = x;
        s >> x;
        length_[i] = x;
        fields_[i] = new SimpleFieldIndex(s);
    }
}

void FieldIndexList::sendTo(eckit::Stream& s) const {

    ASSERT(length_.size() == offset_.size());
    ASSERT(offset_.size() == fields_.size());

    unsigned long count = length_.size();

    s << count;


    for (size_t i = 0; i < count; ++i) {
        unsigned long long o = offset_[i];
        unsigned long long l = length_[i];

        s << o;
        s << l;

        fields_[i]->encode(s);
    }
}

}  // namespace fields
}  // namespace metkit
