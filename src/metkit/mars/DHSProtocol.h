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

#include <iosfwd>
#include <map>
#include <memory>
#include <string>

#include "eckit/io/Length.h"
#include "eckit/net/Endpoint.h"
#include "eckit/net/TCPSocket.h"
#include "eckit/serialisation/Streamable.h"
#include "eckit/types/Types.h"

#include "metkit/mars/BaseProtocol.h"
#include "metkit/mars/ClientTask.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit::mars {

// ---------------------------------------------------------------------------------------------------------------------

class BaseCallbackConnection : public eckit::Streamable {
public:

    BaseCallbackConnection() = default;

    static BaseCallbackConnection* build(const eckit::Configuration& config, const std::string& host = "");

    virtual const eckit::net::Endpoint& endpoint() const = 0;

    virtual eckit::net::TCPSocket& connect() = 0;

    void encode(eckit::Stream&) const override = 0;
    static const eckit::ClassSpec& classSpec();
};

// ---------------------------------------------------------------------------------------------------------------------

class DHSProtocol : public BaseProtocol {

public:

    DHSProtocol(const eckit::Configuration&);

    DHSProtocol(const eckit::Configuration&, const std::map<std::string, std::string>& env);

    DHSProtocol(const std::string& name, const std::string& host, int port, bool forwardMessages = false);

    DHSProtocol(eckit::Stream&);
    ~DHSProtocol() override;

    // -- Overridden methods (from Streamable)

    std::string className() const override { return "DHSProtocol"; }
    const eckit::ReanimatorBase& reanimator() const override;
    static const eckit::ClassSpec& classSpec();
    const eckit::StringDict& stats() const override { return stats_; }

private:

    // -- Members
    std::unique_ptr<BaseCallbackConnection> callback_;
    eckit::net::TCPSocket socket_;
    std::string name_;
    std::string host_;
    int port_;
    std::string msg_;
    std::unique_ptr<ClientTask> task_;
    bool done_;
    bool error_;
    bool sending_;
    bool forward_;
    MarsRequest env_;
    eckit::StringDict stats_;

    // -- Methods
    bool wait(eckit::Length&);

    // -- Overridden methods
    // From BaseProtocol
    eckit::Length retrieve(const MarsRequest& request) override;
    void archive(const MarsRequest& request, const eckit::Length&) override;
    long read(void* buffer, long len) override;
    long write(const void* buffer, long len) override;
    void cleanup() override;
    void print(std::ostream&) const override;
    void encode(eckit::Stream&) const override;
};

// ---------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars

#endif
