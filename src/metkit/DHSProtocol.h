/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File DHSProtocol.h
// Baudouin Raoult - (c) ECMWF Feb 12

#ifndef DHSProtocol_H
#define DHSProtocol_H

#include "metkit/BaseProtocol.h"
#include "eckit/net/TCPServer.h"
#include "eckit/net/TCPSocket.h"
#include "metkit/MarsRequest.h"
#include "metkit/ClientTask.h"
#include "eckit/memory/ScopedPtr.h"

namespace metkit {

class DHSProtocol : public BaseProtocol {

public:

    DHSProtocol(const eckit::Configuration&);

    DHSProtocol(const std::string& name,
                const std::string& host,
                int port,
                bool forewardMessages = false);

    DHSProtocol(eckit::Stream&);
    ~DHSProtocol();

    // -- Overridden methods (from Streamable)

    virtual std::string className() const { return "DHSProtocol"; }
    virtual const eckit::ReanimatorBase& reanimator() const;
    static  const eckit::ClassSpec& classSpec();

private:

    // -- Members
    eckit::TCPServer          callback_;
    eckit::TCPSocket          socket_;
    std::string               name_;
    std::string               host_;
    int                       port_;
    std::string               msg_;
    bool                      done_;
    bool                      error_;
    bool                      sending_;
    eckit::ScopedPtr<ClientTask> task_;
    bool                      foreward_;

    // -- Methods
    bool wait(eckit::Length&);

    // -- Overridden methods
    // From BaseProtocol
    virtual eckit::Length retrieve(const MarsRequest& request);
    virtual void archive(const MarsRequest& request, const eckit::Length&);
    virtual long read(void* buffer, long len);
    virtual long write(const void* buffer, long len);
    virtual void cleanup();
    virtual void print(std::ostream&) const;
    virtual void encode(eckit::Stream&) const;
};

}

#endif
