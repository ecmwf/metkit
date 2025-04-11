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

#ifndef metkit_FieldIndex_H
#define metkit_FieldIndex_H

#include <iosfwd>
#include <vector>


namespace eckit {
class Value;
class JSON;
}  // namespace eckit

namespace metkit {

class MarsRequest;

namespace pointdb {

class DataSource;
class DataSourceHandler;
//----------------------------------------------------------------------------------------------------------------------

struct FieldIndexerStatus {
    size_t count_;
    size_t batch_;
    FieldIndexerStatus(size_t count, size_t batch) : count_(count), batch_(batch) {}
};


class FieldIndexer {
public:

    // -- Exceptions
    // None

    // -- Contructors


    // -- Destructor

    virtual ~FieldIndexer();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual FieldIndexerStatus lookup(const eckit::Value&, DataSourceHandler&) const = 0;

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

    virtual void print(std::ostream& s) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    // -- Members


    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const FieldIndexer& p) {
        p.print(s);
        return s;
    }
};


//----------------------------------------------------------------------------------------------------------------------
}  // namespace pointdb

}  // namespace metkit

#endif
