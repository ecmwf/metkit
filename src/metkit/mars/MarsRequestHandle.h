// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File MarsRequestHandle.h
// Baudouin Raoult - (c) ECMWF Feb 12

#pragma once

#include <memory>
#include <string>

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/types/Types.h"

#include "metkit/mars/BaseProtocol.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit::mars {

class MarsRequestHandle : public eckit::DataHandle {
public:

    class RetryTransfer : public eckit::Exception {
        bool retryOnClient() const override { return true; }

    public:

        RetryTransfer(const std::string& what) : eckit::Exception(what) {}
    };

public:

    MarsRequestHandle(eckit::Stream&);

    MarsRequestHandle(const metkit::mars::MarsRequest& request, const eckit::Configuration& database);

    MarsRequestHandle(const metkit::mars::MarsRequest& request, metkit::mars::BaseProtocol* protocol);

    ~MarsRequestHandle();

    // -- Overridden methods (from Streamable)
    std::string className() const override { return "MarsRequestHandle"; }
    const eckit::ReanimatorBase& reanimator() const override;
    static const eckit::ClassSpec& classSpec();

    const eckit::StringDict& stats() const;

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

}  // namespace metkit::mars
