// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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
