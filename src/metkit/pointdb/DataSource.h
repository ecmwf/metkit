/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef metkit_DataSource_H
#define metkit_DataSource_H

#include <iosfwd>
#include <map>

#include "eckit/memory/Counted.h"

namespace eckit {
class JSON;
class Value;
}

namespace metkit {

class MarsRequest;

namespace pointdb {

class DataSource;

struct PointResult {

    double value_;
    double lat_;
    double lon_;
    const DataSource* source_; // Warning, the source must have a longer life time as the result





    void print(std::ostream& s) const;


    friend std::ostream& operator<<(std::ostream& s, const PointResult& f) {
        f.print(s);
        return s;
    }
};


class DataSource : public eckit::Counted {
public:

    virtual ~DataSource();


    virtual PointResult extract(double lat, double lon) const = 0;

    // Encode a MARS-like request representing the field
    virtual const std::map<std::string, eckit::Value>& request() const = 0;

    // A key to group source togther, e.g. sources poiting to the same file
    virtual std::string groupKey() const = 0;

    // A key to sort sources of the same group, e.g. offset in the file
    virtual std::string sortKey() const = 0;

private:

    virtual void print(std::ostream& s) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const DataSource& f) {
        f.print(s);
        return s;
    }

};


class DataSourceHandle {

    DataSource* source_;

public:

    DataSourceHandle(DataSource* source):
        source_(source) {
        source_->attach();
    }


    DataSourceHandle(const DataSourceHandle& other):
        source_(other.source_) {
        source_->attach();
    }

    DataSourceHandle& operator=(const DataSourceHandle& other) {
        if (source_ != other.source_) {
            source_->detach();
            source_ = other.source_;
            source_->attach();
        }
        return *this;
    }

    ~DataSourceHandle() {
        source_->detach();
    }

    const DataSource& operator*() const { return *source_; }


    const DataSource* operator->() const { return source_; }


    friend std::ostream& operator<<(std::ostream& s, const DataSourceHandle& f) {
        s << *f.source_;
        return s;
    }

};



} // namespace pointdb
} // namespace metkit



#endif
