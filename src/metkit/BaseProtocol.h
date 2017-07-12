/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File BaseProtocol.h
// Baudouin Raoult - (c) ECMWF Feb 12

#ifndef BaseProtocol_H
#define BaseProtocol_H

#include "eckit/io/Length.h"
#include "eckit/serialisation/Streamable.h"

namespace metkit {

class MarsRequest;

class BaseProtocol : public eckit::Streamable {

public:
    BaseProtocol();
    BaseProtocol(eckit::Stream&);
    virtual ~BaseProtocol();

    virtual eckit::Length retrieve(const MarsRequest&) = 0;
    virtual void archive(const MarsRequest&, const eckit::Length&) = 0;

    virtual long read(void* buffer, long len) = 0;
    virtual long write(const void* buffer, long len) = 0;
    virtual void cleanup() = 0;

    // -- Overridden methods (from Streamable)
    static  const eckit::ClassSpec& classSpec();

protected:
    virtual void print(std::ostream&) const = 0;
    virtual void encode(eckit::Stream&) const;

private:

    friend std::ostream& operator<<(std::ostream& s, const BaseProtocol& p) {
        p.print(s); return s;
    }
};

}

#endif
