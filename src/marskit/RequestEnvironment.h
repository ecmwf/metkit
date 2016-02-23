/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File RequestEnvironment.h
// Baudouin Raoult - (c) ECMWF Feb 12

#ifndef RequestEnvironment_H
#define RequestEnvironment_H

#include "eckit/memory/NonCopyable.h"
#include "marskit/MarsRequest.h"

namespace marskit {

class RequestEnvironment : private eckit::NonCopyable {
public:

// -- Contructors

	RequestEnvironment();

// -- Destructor

	~RequestEnvironment();

// -- Methods

    const MarsRequest& request() const { return request_; }

// -- Class members

    static RequestEnvironment& instance();

protected:

// -- Methods

	void print(std::ostream&) const;

private:

// -- Members

    MarsRequest request_;

};

}

#endif
