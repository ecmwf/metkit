/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/TIDEDecoder.h"
#include "eckit/exception/Exceptions.h"
#include "metkit/codes/Message.h"
#include "metkit/mars/MarsRequest.h"

#include <vector>
#include <string>

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------
bool TIDEDecoder::match(const Message& msg) const {
    size_t len = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and p[0] == 'T' and p[1] == 'I' and p[2] == 'D' and p[3] == 'E';
}

mars::MarsRequest TIDEDecoder::messageToRequest(const Message&) const {
    NOTIMP;

}

void TIDEDecoder::print(std::ostream& s) const {
    s << "TIDEDecoder[]";
}


static TIDEDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
