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

#include "eckit/message/MessageContent.h"

typedef struct grib_handle codes_handle;

namespace metkit {
namespace codes {


//----------------------------------------------------------------------------------------------------------------------

class CodesContent : public eckit::message::MessageContent {
public:

    CodesContent(codes_handle* handle, bool delete_handle);
    explicit CodesContent(const codes_handle* handle);

    ~CodesContent();

protected:

    codes_handle* handle_;

    eckit::message::MessageContent* transform(const eckit::StringDict&) const override;

private:

    bool delete_handle_;

    size_t length() const override;
    void write(eckit::DataHandle& handle) const override;
    eckit::DataHandle* readHandle() const override;
    void print(std::ostream& s) const override;
    std::string getString(const std::string& key) const override;
    long getLong(const std::string& key) const override;
    double getDouble(const std::string& key) const override;
    void getDoubleArray(const std::string& key, std::vector<double>& values) const override;
    size_t getSize(const std::string& key) const override;
    void getDoubleArray(const std::string& key, double* data, size_t lenExpected) const override;

    eckit::Offset offset() const override;
    const codes_handle* codesHandle() const;
    const void* data() const override;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
