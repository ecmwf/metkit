/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Manuel Fuentes
/// @author Tiago Quintino

/// @date Dec 2015

#ifndef metkit_BufrToRequest_H
#define metkit_BufrToRequest_H

#include "metkit/bufr/BufrHandle.h"

namespace metkit {

class MarsRequest;

namespace bufr {

//----------------------------------------------------------------------------------------------------------------------

/// Utility class to build MarsRequest from BufrHandle

/// Part of this code is taken from mars-client bufr.c

class BufrToRequest {
public:  // methods

    static void handleToRequest(const bufr::BufrHandle& handle, MarsRequest& req);

    static void messageToRequest(const void* buffer, size_t length, MarsRequest& req);

private:
    BufrToRequest() = delete;
    ~BufrToRequest() = delete;
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace bufr
}  // namespace metkit

#endif
