/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include "eckit/serialisation/Stream.h"

namespace eckit::message {
class Message;
}  // namespace eckit::message

namespace metkit::fields {

class FieldIndex {
public:

    FieldIndex();
    FieldIndex(const eckit::message::Message&);

    FieldIndex(const FieldIndex&)            = delete;
    FieldIndex(FieldIndex&&)                 = delete;
    FieldIndex& operator=(const FieldIndex&) = delete;
    FieldIndex& operator=(FieldIndex&&)      = delete;
    
    FieldIndex(eckit::Stream&);

    virtual ~FieldIndex();

    void getValue(const std::string& name, double& value);
    void getValue(const std::string& name, long& value);
    void getValue(const std::string& name, std::string& value);

    std::string substitute(const std::string& pattern) const;

    void encode(eckit::Stream&) const;

    void setValue(const std::string& name, double value);
    void setValue(const std::string& name, long value);
    void setValue(const std::string& name, const std::string& value);

protected:  // members

    std::map<std::string, std::string> stringValues_;
    std::map<std::string, long> longValues_;
    std::map<std::string, double> doubleValues_;
};

}  // namespace metkit::fields
