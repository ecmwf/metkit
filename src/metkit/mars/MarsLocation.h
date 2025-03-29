/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino

/// @date Dec 2015

#ifndef metkit_MarsLocation_H
#define metkit_MarsLocation_H


#include "eckit/config/Configuration.h"
#include "metkit/mars/MarsRequest.h"

namespace eckit {
class JSON;
class Stream;
}  // namespace eckit

namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

/// MarsLocation represents a MarsRequest associated with the hostname and port.
/// From this location, the data can be directly retrieved without going through a queueing system.
/// It assumes all the data identified by the request can be got from the same location.
/// This is useful to identify and retrieve data directly from memory.
///
/// If we consider that a MarsRequest is analogue to a URI, then a MarsLocation is an analogue to a URL

class MarsLocation {

public:  // methods

    // - Constructors

    MarsLocation(const MarsRequest& r, const std::string& hostname, int port);
    MarsLocation(eckit::Stream&);
    MarsLocation(const eckit::Configuration&);

    // -- Destructor

    ~MarsLocation();

    // -- Operators

    operator eckit::Value() const;

    void json(eckit::JSON&) const;

    const MarsRequest& request() const;

    std::string hostname() const;

    int port() const;


private:  // members

    MarsRequest request_;
    std::string hostname_;
    int port_;

private:  // methods

    void print(std::ostream&) const;
    void encode(eckit::Stream&) const;

    // -- Class members


    friend std::ostream& operator<<(std::ostream& s, const MarsLocation& r) {
        r.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const MarsLocation& r) {
        r.json(s);
        return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream& s, const MarsLocation& r) {
        r.encode(s);
        return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------
}  // namespace mars
}  // namespace metkit

#endif
