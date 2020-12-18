/*
 * (C) Copyright 1996- ECMWF.
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

#include <memory>

#include "eckit/net/TCPServer.h"
#include "eckit/net/TCPSocket.h"

#include "metkit/mars/BaseProtocol.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/ClientTask.h"

namespace metkit {
namespace mars {

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

    virtual std::string className() const override { return "DHSProtocol"; }
    virtual const eckit::ReanimatorBase& reanimator() const;
    static  const eckit::ClassSpec& classSpec();

private:

    // -- Members
    eckit::net::EphemeralTCPServer callback_;
    eckit::net::TCPSocket          socket_;
    std::string               name_;
    std::string               host_;
    int                       port_;
    std::string               msg_;
    std::unique_ptr<ClientTask> task_;
    bool                      done_;
    bool                      error_;
    bool                      sending_;
    bool                      forward_;

    // -- Methods
    bool wait(eckit::Length&);

    // -- Overridden methods
    // From BaseProtocol
    virtual eckit::Length retrieve(const MarsRequest& request) override;
    virtual void archive(const MarsRequest& request, const eckit::Length&) override;
    virtual long read(void* buffer, long len) override;
    virtual long write(const void* buffer, long len) override;
    virtual void cleanup() override;
    virtual void print(std::ostream&) const override;
    virtual void encode(eckit::Stream&) const override;
};

}
}

#endif
