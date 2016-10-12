/*
 * (C) Copyright 1996-2013 ECMWF.
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

#include "metkit/BaseProtocol.h"
#include "metkit/MarsRequest.h"

namespace metkit {

class MarsRequestHandle : public eckit::DataHandle {
public:
	MarsRequestHandle(const metkit::MarsRequest& request, metkit::BaseProtocol* protocol);
	~MarsRequestHandle();


private:
    metkit::MarsRequest request_;
    std::auto_ptr<BaseProtocol> protocol_;

// -- Overridden methods
	// From data handle
	void print(std::ostream&) const;

    eckit::Length openForRead();
    void openForWrite(const eckit::Length&);
    void openForAppend(const eckit::Length&);
    long read(void*, long );
    long write(const void*, long);
    void close();
};

}

#endif
