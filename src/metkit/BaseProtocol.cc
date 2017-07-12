/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File BaseProtocol.cc
// Baudouin Raoult - (c) ECMWF Feb 12


#include "metkit/BaseProtocol.h"

namespace metkit {

BaseProtocol::BaseProtocol() {}

BaseProtocol::BaseProtocol(eckit::Stream&) {}

BaseProtocol::~BaseProtocol() {}

void BaseProtocol::print(std::ostream&) const {}

void BaseProtocol::encode(eckit::Stream&) const {}

}
