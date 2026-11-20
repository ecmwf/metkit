// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File FieldIndex.h
// Baudouin Raoult - ECMWF Oct 19

#pragma once

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
