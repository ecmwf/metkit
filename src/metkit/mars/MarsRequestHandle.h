/*
 * (C) Copyright 1996- ECMWF.
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

#include <memory>

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"

#include "metkit/mars/BaseProtocol.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit {
namespace mars {

class MarsRequestHandle : public eckit::DataHandle {
public:

    class RetryTransfer : public eckit::Exception {
        virtual bool retryOnClient() const { return true; }

    public:

        RetryTransfer(const std::string& what) : eckit::Exception(what) {}
    };

public:

    MarsRequestHandle(eckit::Stream&);

    MarsRequestHandle(const metkit::mars::MarsRequest& request, const eckit::Configuration& database);

    MarsRequestHandle(const metkit::mars::MarsRequest& request, metkit::mars::BaseProtocol* protocol);

    ~MarsRequestHandle();

    // -- Overridden methods (from Streamable)
    virtual std::string className() const override { return "MarsRequestHandle"; }
    virtual const eckit::ReanimatorBase& reanimator() const override;
    static const eckit::ClassSpec& classSpec();

private:  // members

    metkit::mars::MarsRequest request_;

    std::unique_ptr<BaseProtocol> protocol_;

    bool opened_;

private:  // members

    void print(std::ostream&) const override;
    void encode(eckit::Stream&) const override;

    eckit::Length openForRead() override;
    void openForWrite(const eckit::Length&) override;
    void openForAppend(const eckit::Length&) override;
    long read(void*, long) override;
    long write(const void*, long) override;
    void close() override;
    bool canSeek() const override;
};

}  // namespace mars
}  // namespace metkit

#endif
