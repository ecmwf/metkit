// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/fields/SimpleFieldIndex.h"

namespace metkit::fields {

SimpleFieldIndex::SimpleFieldIndex(eckit::Stream& s) : FieldIndex(s) {}

SimpleFieldIndex::~SimpleFieldIndex() {}

}  // namespace metkit::fields
