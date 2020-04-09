/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/MarsExpandContext.h"
#include "metkit/types/Type.h"


namespace metkit {

MarsExpandContext::~MarsExpandContext() {}

void DummyContext::info(std::ostream &) const {}

} // namespace metkit
