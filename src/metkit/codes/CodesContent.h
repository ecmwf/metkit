/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jun 2020

#pragma once

#include "eccodes.h"

#include "eckit/message/MessageContent.h"

typedef struct grib_handle codes_handle;

namespace metkit {
namespace codes {

class CodesContent : public eckit::message::MessageContent {
public:
    CodesContent(codes_handle* handle, bool delete_handle);
    explicit CodesContent(const codes_handle* handle);

    ~CodesContent();

private:
    codes_handle* handle_;
    bool delete_handle_;

    virtual size_t length() const override;
    virtual void write(eckit::DataHandle& handle) const override;
    eckit::DataHandle* readHandle() const override;
    virtual void print(std::ostream & s) const override;
    virtual std::string getString(const std::string& key) const override;
    virtual long getLong(const std::string& key) const override;
    virtual double getDouble(const std::string& key) const override;
    virtual void getDoubleArray(const std::string& key, std::vector<double>& values) const override;
    virtual eckit::Offset offset() const override;
    const codes_handle* codesHandle() const;
    const void* data() const override;

    eckit::message::MessageContent* transform(const eckit::StringDict&) const override;
};

}  // namespace codes
}  // namespace metkit
