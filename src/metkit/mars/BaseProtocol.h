/*
 * (C) Copyright 1996- ECMWF.
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

namespace eckit {
class Configuration;
}

namespace metkit {
namespace mars {

class MarsRequest;

class BaseProtocol : public eckit::Streamable {

public:

    BaseProtocol();
    BaseProtocol(eckit::Stream&);
    BaseProtocol(const eckit::Configuration&);

    virtual ~BaseProtocol() override;

    virtual eckit::Length retrieve(const MarsRequest&)             = 0;
    virtual void archive(const MarsRequest&, const eckit::Length&) = 0;

    virtual long read(void* buffer, long len)        = 0;
    virtual long write(const void* buffer, long len) = 0;
    virtual void cleanup()                           = 0;

    // -- Overridden methods (from Streamable)
    static const eckit::ClassSpec& classSpec();

protected:

    virtual void print(std::ostream&) const = 0;
    virtual void encode(eckit::Stream&) const override;

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
    virtual BaseProtocol* make(const eckit::Configuration& param) override { return new T(param); }

public:

    ProtocolBuilder(const std::string& name) : ProtocolFactory(name) {}
};

}  // namespace mars
}  // namespace metkit

#endif
