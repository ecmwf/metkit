/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include <iosfwd>
#include <map>

namespace eckit {
class JSON;
class Value;
}  // namespace eckit

namespace metkit {

class MarsRequest;

namespace pointdb {

class DataSource;

struct PointResult {

    double value_;
    double lat_;
    double lon_;
    const DataSource* source_;  // Warning, the source must have a longer life time as the result


    void print(std::ostream& s) const;


    friend std::ostream& operator<<(std::ostream& s, const PointResult& f) {
        f.print(s);
        return s;
    }
};


class DataSourceHandler {
public:

    virtual void handle(DataSource*) = 0;
};

class DataSource {
public:

    DataSource()          = default;
    virtual ~DataSource() = default;

    DataSource(const DataSource&)            = delete;
    DataSource(DataSource&&)                 = delete;
    DataSource& operator=(const DataSource&) = delete;
    DataSource& operator=(DataSource&&)      = delete;

    virtual PointResult extract(double lat, double lon) const = 0;

    // Encode a MARS-like request representing the field
    virtual const std::map<std::string, eckit::Value>& request() const = 0;

    // A key to group source togther, e.g. sources poiting to the same file
    virtual std::string groupKey() const = 0;

    // A key to sort sources of the same group, e.g. offset in the file
    virtual std::string sortKey() const = 0;

    // Used to throw away requests in case of restarted transactions
    virtual size_t batch() const;

private:

    virtual void print(std::ostream& s) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const DataSource& f) {
        f.print(s);
        return s;
    }
};


}  // namespace pointdb
}  // namespace metkit
