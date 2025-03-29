/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/fields/SimpleFieldIndex.h"

namespace metkit {
namespace fields {

SimpleFieldIndex::SimpleFieldIndex(eckit::Stream& s) : FieldIndex(s) {}

SimpleFieldIndex::~SimpleFieldIndex() {}

}  // namespace fields
}  // namespace metkit
