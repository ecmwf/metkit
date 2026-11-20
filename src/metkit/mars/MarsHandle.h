// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File MarsHandle.h
// Baudouin Raoult - ECMWF Oct 96

#pragma once

#include "eckit/io/Length.h"
#include "eckit/io/TCPHandle.h"
#include "eckit/serialisation/Stream.h"

class MarsHandle : public eckit::TCPHandle {
public:

    // -- Contructors

    MarsHandle(const std::string& host, int port, unsigned long long);
    MarsHandle(eckit::Stream&);

    // -- Destructor

    ~MarsHandle();

    // -- Overridden methods

    // From eckit::DataHandle

    virtual eckit::Length openForRead() override;
    virtual void openForWrite(const eckit::Length&) override;
    virtual void openForAppend(const eckit::Length&) override;

    virtual void close() override;
    virtual long read(void*, long) override;
    virtual long write(const void*, long) override;

    virtual eckit::Length estimate() override;
    virtual std::string title() const override;
    virtual std::string metricsTag() const override;

    virtual bool moveable() const override { return true; }

    // From Streamable

    virtual void encode(eckit::Stream&) const override;
    virtual const eckit::ReanimatorBase& reanimator() const override { return reanimator_; }

    // -- Class methods

    static const eckit::ClassSpec& classSpec();

private:

    // -- Members

    unsigned long long clientID_;
    eckit::Length length_;
    eckit::Length total_;
    bool receiving_;
    bool streamMode_;
    bool doCRC_;
    unsigned long crc_;

    // -- Methods

    void updateCRC(void*, long);

    // -- Class members

    static eckit::Reanimator<MarsHandle> reanimator_;

    friend class MarsHandleStream;
};
