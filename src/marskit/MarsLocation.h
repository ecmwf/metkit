/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino

/// @date Dec 2015

#ifndef marskit_MarsLocation_H
#define marskit_MarsLocation_H

#include "eckit/serialisation/Streamable.h"

#include "marskit/MarsRequest.h"

namespace eckit {
    class JSON;
    class Configuration;
}

namespace marskit {

//----------------------------------------------------------------------------------------------------------------------

/// MarsLocation represents a MarsRequest associated with the hostname and port.
/// From this location, the data can be directly retrieved without going through a queueing system.
/// It assumes all the data identified by the request can be got from the same location.
/// This is useful to identify and retrieve data directly from memory.
///
/// If we consider that a MarsRequest is analogue to a URI, then a MarsLocation is an analogue to a URL

class MarsLocation : public eckit::Streamable {

public: // methods

// - Constructors

    MarsLocation(const eckit::Configuration&);
    MarsLocation(const MarsRequest& r, const std::string& hostname, int port);
    MarsLocation(eckit::Stream&);

// -- Destructor

    virtual ~MarsLocation();

// -- Operators

    operator eckit::Value() const;

    void json(eckit::JSON&) const;

    const MarsRequest& request() const;

    std::string hostname() const;

    int port() const;

    // Overridden from Streamble

    virtual void encode(eckit::Stream&) const;
    virtual const eckit::ReanimatorBase& reanimator() const { return reanimator_; }

    static  const eckit::ClassSpec&  classSpec()        { return classSpec_;}

private: // members

    MarsRequest 	request_;
    std::string     hostname_;
    int             port_;

private: // methods

	void print(std::ostream&) const;

// -- Class members

    static eckit::ClassSpec                  classSpec_;
    static eckit::Reanimator<MarsLocation>   reanimator_;

    friend std::ostream& operator<<(std::ostream& s, const MarsLocation& r) {
        r.print(s); return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const MarsLocation& r) {
        r.json(s); return s;
    }
};

//----------------------------------------------------------------------------------------------------------------------

} // namespace marskit

#endif
