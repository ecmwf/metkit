/*
 * (C) Copyright 1996-2013 ECMWF.
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

#include "marskit/BaseProtocol.h"
#include "eckit/net/TCPServer.h"
#include "eckit/net/TCPSocket.h"
#include "marskit/MarsRequest.h"
#include "marskit/ClientTask.h"

namespace marskit {

class DHSProtocol : public BaseProtocol {
public:
	DHSProtocol(const std::string& name, const std::string& host, int port);
	~DHSProtocol();

private:
// No copy allowed
	DHSProtocol(const DHSProtocol&);
	DHSProtocol& operator=(const DHSProtocol&);

// -- Members
    eckit::TCPServer   callback_;
    eckit::TCPSocket   socket_;
    std::string      name_;
    std::string      host_;
    int         port_;
    std::string      msg_;
    bool        done_;
    bool        error_;
    bool        sending_;
    std::auto_ptr<ClientTask> task_;

// -- Methods
    bool wait(eckit::Length&);

// -- Overridden methods
	// From BaseProtocol
    eckit::Length retrieve(const MarsRequest& request);
    void archive(const MarsRequest& request, const eckit::Length&);
    long read(void* buffer, long len);
    long write(const void* buffer, long len);
    void cleanup();
	void print(std::ostream&) const;

// -- Friends
	//friend std::ostream& operator<<(std::ostream& s,const DHSProtocol& p)
	//	{ p.print(s); return s; }
};

}

#endif
