/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File MarsRequestHandle.h
// Baudouin Raoult - (c) ECMWF Feb 12

#ifndef MarsRequestHandle_H
#define MarsRequestHandle_H

#include "eckit/io/DataHandle.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/exception/Exceptions.h"

#include "metkit/BaseProtocol.h"
#include "metkit/MarsRequest.h"

namespace metkit {

class MarsRequestHandle : public eckit::DataHandle {
public:

    class RetryTransfer : public eckit::Exception {
        virtual bool retryOnClient() const { return true; }
    public:
        RetryTransfer(const std::string& what):
            eckit::Exception(what) {}
    };

public:

    MarsRequestHandle(eckit::Stream&);

    MarsRequestHandle(const metkit::MarsRequest& request,
                      const eckit::Configuration& database);

    MarsRequestHandle(const metkit::MarsRequest& request,
                      metkit::BaseProtocol* protocol);

    ~MarsRequestHandle();

    // -- Overridden methods (from Streamable)
    virtual std::string className() const { return "MarsRequestHandle"; }
    virtual const eckit::ReanimatorBase& reanimator() const;
    static  const eckit::ClassSpec& classSpec();

private:
    // -- Members
    metkit::MarsRequest request_;
    eckit::ScopedPtr<BaseProtocol> protocol_;

    bool opened_;

    // -- Overridden methods
    // From data handle
    void print(std::ostream&) const;
    void encode(eckit::Stream&) const;

    eckit::Length openForRead();
    void openForWrite(const eckit::Length&);
    void openForAppend(const eckit::Length&);
    long read(void*, long );
    long write(const void*, long);
    void close();
};

}

#endif
