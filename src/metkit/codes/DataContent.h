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



#ifndef mars_client_DataContent_H
#define mars_client_DataContent_H

#include "metkit/codes/MessageContent.h"


namespace metkit {
namespace codes {

class DataContent : public MessageContent {
public:

    DataContent(const void* data, size_t size);
    ~DataContent();

private:

    const void* data_;
    size_t size_;
    mutable codes_handle* handle_;

    virtual eckit::DataHandle* readHandle() const;
    virtual size_t length() const;
    virtual const void* data() const;
    virtual void write(eckit::DataHandle& handle) const;
    virtual const codes_handle* codesHandle() const;


};


}  // namespace codes
}  // namespace metkit


#endif
