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

#include "eckit/memory/Counted.h"

#include "metkit/codes/Message.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"

#include <eccodes.h>

namespace metkit {
namespace codes {

class MessageContent : public eckit::Counted {

public:

    virtual operator bool() const {
        return true;
    }

    virtual void write(eckit::DataHandle&) const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " write()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual size_t length() const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " length(get)";
        throw eckit::SeriousBug(oss.str());
    }

    virtual std::string getString(const std::string& key) const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " getString()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual long getLong(const std::string& key) const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " getLong()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual double getDouble(const std::string& key) const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " getDouble()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual void getDoubleArray(const std::string& key, std::vector<double>&) const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " getDoubleArray()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual eckit::DataHandle* readHandle() const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " readHandle()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual eckit::DataHandle* writeHandle() const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " writeHandle()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual eckit::Offset offset() const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " offset()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual const void* data() const{
        std::ostringstream oss;
        oss << "Not implemented " << *this << " data()";
        throw eckit::SeriousBug(oss.str());
    }

    virtual const codes_handle* codesHandle() const {
        std::ostringstream oss;
        oss << "Not implemented " << *this << " codesHandle()";
        throw eckit::SeriousBug(oss.str());
    }

private:

    virtual void print(std::ostream &) const = 0;

    friend std::ostream &operator<<(std::ostream &s, const MessageContent &p) {
        p.print(s);
        return s;
    }
};

class NoContent : public MessageContent {

    virtual operator bool() const {
        return false;
    }

    void print(std::ostream & s) const {
        s << "NoContent[]";
    }
};

class CodesContent : public MessageContent {
public:
    CodesContent(codes_handle* handle, bool delete_handle):
        handle_(handle),
        delete_handle_(delete_handle) {

    }
    ~CodesContent() {
        if (delete_handle_) {
            codes_handle_delete(handle_);
        }
    }

private:
    codes_handle* handle_;
    bool delete_handle_;

    virtual size_t length() const {
        size_t size;
        const void* data;
        ASSERT(codes_get_message(handle_, &data, &size) == 0);
        return size;
    }

    virtual void write(eckit::DataHandle& handle) const {
        size_t size;
        const void* data;
        ASSERT(codes_get_message(handle_, &data, &size) == 0);
        ASSERT(handle.write(data, size) == size);

    }

    eckit::DataHandle* readHandle() const {
        size_t size;
        const void* data;
        ASSERT(codes_get_message(handle_, &data, &size) == 0);
        return new eckit::MemoryHandle(data, size);
    }

    virtual void print(std::ostream & s) const {
        s << "CodesContent[]";
    }

    virtual std::string getString(const std::string& key) const {
        char values[10240];
        size_t len = sizeof(values);

        values[0] = 0;

        int err = codes_get_string(handle_, key.c_str(), values, &len);
        // ASSERT(err)

        return values;
    }

    virtual long getLong(const std::string& key) const {
        long v = 0;
        int err = codes_get_long(handle_, key.c_str(), &v);
        return v;
    }

    virtual double getDouble(const std::string& key) const {
        double v = 0;
        int err = codes_get_double(handle_, key.c_str(), &v);
        return v;
    }

    virtual void getDoubleArray(const std::string& key, std::vector<double>& values) const {
        size_t size = 0;
        int err = codes_get_size(handle_, key.c_str(), &size);

        size_t count = size;
        values.resize(count);
        ASSERT(err == 0);
        err = codes_get_double_array(handle_, key.c_str(), &values[0], &count);
        ASSERT(err == 0);
        ASSERT(count == size);
    }

    virtual eckit::Offset offset() const {
        long pos;
        ASSERT(codes_get_long(handle_, "offset", &pos) == 0);
        return pos;
    }

    const codes_handle* codesHandle() const {
        return handle_;
    }

    const void* data() const {
        size_t size;
        const void* data;
        ASSERT(codes_get_message(handle_, &data, &size) == 0);
        return data;
    }

};

class UserDataContent : public MessageContent {

    const void* data_;
    size_t size_;

    void print(std::ostream & s) const {
        s << "UserDataContent[]";
    }

    eckit::DataHandle* readHandle() const {
        return new eckit::MemoryHandle(data_, size_);
    }

    virtual size_t length() const {
        return size_;
    }

    const void* data() const {
        return data_;
    }

    virtual void write(eckit::DataHandle& handle) const {
        ASSERT(handle.write(data_, size_) == size_);
    }

public:
    UserDataContent(const void* data, size_t size):
        data_(data),
        size_(size) {
    }

};

Message::Message():
    content_(new NoContent()) {
    content_->attach();
}

Message::Message(const void* data, size_t size):
    content_(new UserDataContent(data, size)) {
    content_->attach();
}

Message::Message(codes_handle* handle, bool del):
    content_(handle ?
             static_cast<MessageContent*>(new CodesContent(handle, del)) :
             static_cast<MessageContent*>(new NoContent())) {
    content_->attach();
}

Message& Message::operator=(const Message& other) {
    if (content_ != other.content_) {
        content_->detach();
        content_ = other.content_;
        content_->attach();
    }
    return *this;
}

Message::~Message() {
    content_->detach();
}

void Message::print(std::ostream &s) const {
    s << "Message[]";
}

Message::operator bool() const {
    return content_->operator bool();
}

void Message::write(eckit::DataHandle& handle) const {
    content_->write(handle);
}

size_t Message::length() const {
    return content_->length();
}

std::string Message::getString(const std::string& key) const {
    return content_->getString(key);
}

long Message::getLong(const std::string& key) const {
    return content_->getLong(key);
}

double Message::getDouble(const std::string& key) const {
    return content_->getDouble(key);
}

void Message::getDoubleArray(const std::string& key, std::vector<double>& v) const {
    return content_->getDoubleArray(key, v);
}

eckit::DataHandle* Message::readHandle() const {
    return content_->readHandle();
}

eckit::DataHandle* Message::writeHandle() const {
    return content_->writeHandle();
}

eckit::Offset Message::offset() const {
    return content_->offset();
}

const codes_handle* Message::codesHandle() const {
    return content_->codesHandle();
}

const void* Message::data() const {
    return content_->data();
}

}  // namespace close
}  // namespace metkit

