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



#ifndef mars_client_Message_H
#define mars_client_Message_H

#include <iosfwd>
#include <vector>

// #include "eckit/memory/Counted.h"
typedef struct grib_handle codes_handle;

namespace eckit {
class DataHandle;
class Offset;
class PathName;
};

namespace metkit {
namespace codes {

class MessageContent;


class Message {
public:

    Message();

    Message(codes_handle* handle, bool del=true);

    Message(const Message&);
    Message(const void* data, size_t len);


    ~Message();
    Message& operator=(const Message&);

    operator bool() const;

    void write(eckit::DataHandle&) const;

    size_t length() const;
    eckit::Offset offset() const;
    const void* data() const;

    std::string getString(const std::string& key) const;
    long getLong(const std::string& key) const;
    double getDouble(const std::string& key) const;
    void getDoubleArray(const std::string& key, std::vector<double>&) const;

    const codes_handle* codesHandle() const;
    eckit::DataHandle* readHandle() const;
    eckit::DataHandle* writeHandle() const;

private:

    mutable MessageContent* content_;

    void print(std::ostream &) const; // Change to virtual if base class

    friend std::ostream &operator<<(std::ostream &s, const Message &p) {
        p.print(s);
        return s;
    }
};


}  // namespace codes
}  // namespace metkit


#endif
