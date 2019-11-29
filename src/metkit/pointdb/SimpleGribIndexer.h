/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File FieldIndex.h
// Baudouin Raoult - ECMWF Oct 19

#ifndef metkit_SimpleGribIndexer_H
#define metkit_SimpleGribIndexer_H

#include "metkit/pointdb/FieldIndexer.h"
#include "eckit/filesystem/PathName.h"



namespace metkit {
namespace pointdb {


//----------------------------------------------------------------------------------------------------------------------

class SimpleGribIndexer: public FieldIndexer {
public:

// -- Exceptions
	// None

// -- Contructors

	SimpleGribIndexer(const eckit::PathName& path);

// -- Destructor

// -- Convertors
	// None

// -- Operators
	// None

// -- Methods

	virtual std::vector<DataSourceHandle> lookup(const eckit::Value& ) const;

	virtual void summary(eckit::JSON& json) const;

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

	// Uncomment for persistent, remove otherwise

protected:

// -- Members
	// None

// -- Methods

	virtual void print(std::ostream& s) const;

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

private:

// No copy allowed

// -- Members

	eckit::PathName path_;

// -- Methods


	void scan(const eckit::PathName& path) const;

// -- Overridden methods
	// None

// -- Class members
	// None

// -- Class methods
	// None

// -- Friends


};



//----------------------------------------------------------------------------------------------------------------------
} // namespace pointdb

} // namespace metkit

#endif
