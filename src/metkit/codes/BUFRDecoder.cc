/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/BUFRDecoder.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/codes/Message.h"
#include "metkit/mars/MarsRequest.h"

#include <vector>
#include <string>

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------
bool BUFRDecoder::match(const Message& msg) const {
    size_t len = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and p[0] == 'B' and p[1] == 'U' and p[2] == 'F' and p[3] == 'R';
}

mars::MarsRequest BUFRDecoder::messageToRequest(const Message&) const {
    NOTIMP;

}

void BUFRDecoder::print(std::ostream& s) const {
    s << "BUFRDecoder[]";
}


static BUFRDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
