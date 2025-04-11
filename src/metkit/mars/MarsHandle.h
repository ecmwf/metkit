/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File MarsHandle.h
// Baudouin Raoult - ECMWF Oct 96

#ifndef MarsHandle_H
#define MarsHandle_H

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

#endif
