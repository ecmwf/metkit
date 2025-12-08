/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/CodesContent.h"

#include "eccodes.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/io/DataHandle.h"
#include "eckit/io/MemoryHandle.h"

#include "metkit/codes/GribHandle.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

CodesContent::CodesContent(std::unique_ptr<CodesHandle> handle) : handle_(std::move(handle)) {}


//----------------------------------------------------------------------------------------------------------------------

size_t CodesContent::length() const {
    return handle_->messageSize();
}


//----------------------------------------------------------------------------------------------------------------------

void CodesContent::write(eckit::DataHandle& handle) const {
    auto data = handle_->messageData();
    if (handle.write(data.data(), data.size()) != data.size()) {
        std::ostringstream oss;
        oss << "Write error to data handle " << handle;
        throw eckit::WriteError(oss.str(), Here());
    }
}


//----------------------------------------------------------------------------------------------------------------------

eckit::DataHandle* CodesContent::readHandle() const {
    auto data = handle_->messageData();
    return new eckit::MemoryHandle(data.data(), data.size());
}


//----------------------------------------------------------------------------------------------------------------------

void CodesContent::print(std::ostream& s) const {
    s << "CodesContent[]";
}


//----------------------------------------------------------------------------------------------------------------------

std::string CodesContent::getString(const std::string& key) const {
    return handle_->getString(key);
}

//----------------------------------------------------------------------------------------------------------------------

long CodesContent::getLong(const std::string& key) const {
    return handle_->getLong(key);
}


//----------------------------------------------------------------------------------------------------------------------

double CodesContent::getDouble(const std::string& key) const {
    return handle_->getDouble(key);
}


//----------------------------------------------------------------------------------------------------------------------

void CodesContent::getDoubleArray(const std::string& key, std::vector<double>& values) const {
    values = handle_->getDoubleArray(key);
}

//----------------------------------------------------------------------------------------------------------------------

void CodesContent::getFloatArray(const std::string& key, std::vector<float>& values) const {
    values = handle_->getFloatArray(key);
}


//----------------------------------------------------------------------------------------------------------------------

size_t CodesContent::getSize(const std::string& key) const {
    return handle_->size(key);
}


//----------------------------------------------------------------------------------------------------------------------

void CodesContent::getDoubleArray(const std::string& key, double* data, size_t len) const {
    auto arr = handle_->getDoubleArray(key);
    ASSERT(len == arr.size());
    std::copy(arr.begin(), arr.end(), data);
}

//----------------------------------------------------------------------------------------------------------------------

void CodesContent::getFloatArray(const std::string& key, float* data, size_t len) const {
    auto arr = handle_->getFloatArray(key);
    ASSERT(len == arr.size());
    std::copy(arr.begin(), arr.end(), data);
}


//----------------------------------------------------------------------------------------------------------------------

void CodesContent::transform(const eckit::OrderedStringDict& dict) {
    for (auto& [key, value] : dict) {
        handle_->set(key, value);
    }
}

//----------------------------------------------------------------------------------------------------------------------

eckit::Offset CodesContent::offset() const {
    return handle_->getLong("offset");
}


//----------------------------------------------------------------------------------------------------------------------

const void* CodesContent::data() const {
    return handle_->messageData().data();
}


}  // namespace codes
}  // namespace metkit
