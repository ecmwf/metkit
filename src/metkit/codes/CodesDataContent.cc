/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/CodesDataContent.h"

#include "eccodes.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"

#include "metkit/codes/GribHandle.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

CodesDataContent::CodesDataContent(std::unique_ptr<CodesHandle> handle) :
    handle_(std::move(handle)), offset_(handle_->getLong("offset")) {}


CodesDataContent::CodesDataContent(std::unique_ptr<CodesHandle> handle, eckit::Offset offset) :
    handle_(std::move(handle)), offset_(std::move(offset)) {}


//----------------------------------------------------------------------------------------------------------------------

size_t CodesDataContent::length() const {
    return handle_->messageSize();
}


//----------------------------------------------------------------------------------------------------------------------

void CodesDataContent::write(eckit::DataHandle& handle) const {
    auto data = handle_->messageData();
    if (handle.write(data.data(), data.size()) != data.size()) {
        std::ostringstream oss;
        oss << "Write error to data handle " << handle;
        throw eckit::WriteError(oss.str(), Here());
    }
}


//----------------------------------------------------------------------------------------------------------------------

eckit::DataHandle* CodesDataContent::readHandle() const {
    auto data = handle_->messageData();
    return new eckit::MemoryHandle(data.data(), data.size());
}


//----------------------------------------------------------------------------------------------------------------------

void CodesDataContent::print(std::ostream& s) const {
    s << "CodesDataContent[]";
}


//----------------------------------------------------------------------------------------------------------------------

std::string CodesDataContent::getString(const std::string& key) const {
    return handle_->getString(key);
}

//----------------------------------------------------------------------------------------------------------------------

long CodesDataContent::getLong(const std::string& key) const {
    return handle_->getLong(key);
}


//----------------------------------------------------------------------------------------------------------------------

double CodesDataContent::getDouble(const std::string& key) const {
    return handle_->getDouble(key);
}


//----------------------------------------------------------------------------------------------------------------------

void CodesDataContent::getDoubleArray(const std::string& key, std::vector<double>& values) const {
    values = handle_->getDoubleArray(key);
}

//----------------------------------------------------------------------------------------------------------------------

void CodesDataContent::getFloatArray(const std::string& key, std::vector<float>& values) const {
    values = handle_->getFloatArray(key);
}


//----------------------------------------------------------------------------------------------------------------------

size_t CodesDataContent::getSize(const std::string& key) const {
    return handle_->size(key);
}


//----------------------------------------------------------------------------------------------------------------------

void CodesDataContent::getDoubleArray(const std::string& key, double* data, size_t len) const {
    auto arr = handle_->getDoubleArray(key);
    ASSERT(len == arr.size());
    std::copy(arr.begin(), arr.end(), data);
}

//----------------------------------------------------------------------------------------------------------------------

void CodesDataContent::getFloatArray(const std::string& key, float* data, size_t len) const {
    auto arr = handle_->getFloatArray(key);
    ASSERT(len == arr.size());
    std::copy(arr.begin(), arr.end(), data);
}


//----------------------------------------------------------------------------------------------------------------------

void CodesDataContent::transform(const eckit::OrderedStringDict& dict) {
    for (auto& [key, value] : dict) {
        handle_->set(key, value);
    }
}

//----------------------------------------------------------------------------------------------------------------------

eckit::Offset CodesDataContent::offset() const {
    return offset_;
}

//----------------------------------------------------------------------------------------------------------------------

const void* CodesDataContent::data() const {
    return handle_->messageData().data();
}

//----------------------------------------------------------------------------------------------------------------------

const CodesHandle& CodesDataContent::codesHandle() const {
    return *handle_.get();
}

CodesHandle& CodesDataContent::codesHandle() {
    return *handle_.get();
}


}  // namespace codes
}  // namespace metkit
