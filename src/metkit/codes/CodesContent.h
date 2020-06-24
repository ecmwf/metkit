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



#ifndef mars_client_CodesContent_H
#define mars_client_CodesContent_H

#include "metkit/codes/MessageContent.h"

namespace metkit {
namespace codes {

class CodesContent : public MessageContent {
public:
    CodesContent(codes_handle* handle, bool delete_handle);
    CodesContent(const codes_handle* handle);

    ~CodesContent();

private:
    codes_handle* handle_;
    bool delete_handle_;

    virtual size_t length() const;
    virtual void write(eckit::DataHandle& handle) const;
    eckit::DataHandle* readHandle() const;
    virtual void print(std::ostream & s) const;
    virtual std::string getString(const std::string& key) const;
    virtual long getLong(const std::string& key) const;
    virtual double getDouble(const std::string& key) const;
    virtual void getDoubleArray(const std::string& key, std::vector<double>& values) const;
    virtual eckit::Offset offset() const;
    const codes_handle* codesHandle() const;
    const void* data() const;

};


}  // namespace codes
}  // namespace metkit


#endif
