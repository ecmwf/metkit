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

#include <iostream>

#include "metkit/codes/Message.h"
#include "metkit/codes/MessageContent.h"
#include "eckit/io/Offset.h"

// #include "eckit/exception/Exceptions.h"
// #include "eckit/io/DataHandle.h"
// #include "eckit/io/MemoryHandle.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/codes/Decoder.h"
#include "metkit/codes/UserDataContent.h"
#include "metkit/codes/CodesContent.h"



namespace metkit {
namespace codes {

class NoContent : public MessageContent {

    virtual operator bool() const {
        return false;
    }

    void print(std::ostream & s) const {
        s << "NoContent[]";
    }
};


Message::Message():
    content_(new NoContent()) {
    content_->attach();
}

Message::Message(const Message& other):
    content_(other.content_) {
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

Message::Message(const codes_handle* handle):
    content_(handle ?
             static_cast<MessageContent*>(new CodesContent(const_cast<codes_handle*>(handle), false)) :
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

eckit::Offset Message::offset() const {
    return content_->offset();
}

const codes_handle* Message::codesHandle() const {
    return content_->codesHandle();
}

const void* Message::data() const {
    return content_->data();
}

mars::MarsRequest Message::request() const {
    return Decoder::lookup(*this).messageToRequest(*this);
}


}  // namespace close
}  // namespace metkit

