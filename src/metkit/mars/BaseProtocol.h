// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File BaseProtocol.h
// Baudouin Raoult - (c) ECMWF Feb 12

#pragma once

#include <string>

#include "eckit/io/Length.h"
#include "eckit/serialisation/Streamable.h"
#include "eckit/types/Types.h"

namespace eckit {
class Configuration;
}

namespace metkit::mars {

class MarsRequest;

class BaseProtocol : public eckit::Streamable {

public:

    BaseProtocol();
    BaseProtocol(eckit::Stream&);
    BaseProtocol(const eckit::Configuration&);

    virtual eckit::Length retrieve(const MarsRequest&)             = 0;
    virtual void archive(const MarsRequest&, const eckit::Length&) = 0;

    virtual long read(void* buffer, long len)        = 0;
    virtual long write(const void* buffer, long len) = 0;
    virtual void cleanup()                           = 0;

    virtual const eckit::StringDict& stats() const;

    // -- Overridden methods (from Streamable)
    static const eckit::ClassSpec& classSpec();

protected:

    virtual void print(std::ostream&) const = 0;
    void encode(eckit::Stream&) const override;

private:

    friend std::ostream& operator<<(std::ostream& s, const BaseProtocol& p) {
        p.print(s);
        return s;
    }
};


class ProtocolFactory {

    std::string name_;

    virtual BaseProtocol* make(const eckit::Configuration&) = 0;

protected:

    ProtocolFactory(const std::string&);

    virtual ~ProtocolFactory();

public:

    static BaseProtocol* build(const eckit::Configuration&);

    static void list(std::ostream&);
};


template <class T>
class ProtocolBuilder : public ProtocolFactory {
    BaseProtocol* make(const eckit::Configuration& param) override { return new T(param); }

public:

    ProtocolBuilder(const std::string& name) : ProtocolFactory(name) {}
};

}  // namespace metkit::mars
