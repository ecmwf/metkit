/*
 * (C) Copyright 1996-2017 ECMWF.
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
#include "eckit/serialisation/Stream.h"
#include "eckit/io/TCPHandle.h"

class MarsHandle : public eckit::TCPHandle {
public:

// -- Contructors

	MarsHandle(const std::string& host, int port, unsigned long long);
	MarsHandle(eckit::Stream&);

// -- Destructor

	~MarsHandle();

// -- Overridden methods

	// From eckit::DataHandle

    virtual eckit::Length openForRead();
    virtual void openForWrite(const eckit::Length&);
    virtual void openForAppend(const eckit::Length&);

	virtual void close();
	virtual long read(void*,long);
	virtual long write(const void*,long);

	virtual eckit::Length estimate();
	virtual std::string title() const;
    virtual bool moveable() const { return true; }

	// From Streamable

	virtual void encode(eckit::Stream&) const;
	virtual const eckit::ReanimatorBase& reanimator() const { return reanimator_; }

// -- Class methods

	static  const eckit::ClassSpec&  classSpec()         { return classSpec_;}

private:

// -- Members

	unsigned long long  clientID_;
	eckit::Length         length_;
	eckit::Length         total_;
	bool           receiving_;
	bool           streamMode_;
	bool           doCRC_;
	unsigned long  crc_;

// -- Methods

	void updateCRC(void*,long);

// -- Class members

    static  eckit::ClassSpec               classSpec_;
	static  eckit::Reanimator<MarsHandle>  reanimator_;

	friend class MarsHandleStream;

};

#endif
