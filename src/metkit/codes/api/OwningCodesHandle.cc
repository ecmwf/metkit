/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/api/OwningCodesHandle.h"

#include "eccodes.h"

namespace metkit::codes {

namespace {
void checkCodes(int code) {
    if (code != 0) {
        std::string msg(codes_get_error_message(code));
        throw CodesException(msg);
    }
};

codes_handle* castToCodes(CodesHandlePtr* ptr) {
    return static_cast<codes_handle*>(static_cast<void*>(ptr));
}
}  // namespace

OwningCodesHandle::OwningCodesHandle(CodesHandlePtr* handle) : handle_{handle}, delegate_{handle} {}
OwningCodesHandle::~OwningCodesHandle() {
    if (handle_ != NULL) {
        checkCodes(codes_handle_delete(castToCodes(handle_)));
        handle_ = NULL;
    }
}


size_t OwningCodesHandle::messageSize() const {
    return delegate_.messageSize();
}

bool OwningCodesHandle::isDefined(const std::string& key) const {
    return delegate_.isDefined(key);
};

bool OwningCodesHandle::isMissing(const std::string& key) const {
    return delegate_.isMissing(key);
};

bool OwningCodesHandle::has(const std::string& key) const {
    return delegate_.has(key);
};

void OwningCodesHandle::setMissing(const std::string& key) {
    delegate_.setMissing(key);
};

void OwningCodesHandle::set(const std::string& key, const std::string& value) {
    return delegate_.set(key, value);
};
void OwningCodesHandle::set(const std::string& key, double value) {
    return delegate_.set(key, value);
};
void OwningCodesHandle::set(const std::string& key, long value) {
    return delegate_.set(key, value);
};

void OwningCodesHandle::set(const std::string& key, Span<const std::string> value) {
    delegate_.set(key, value);
};
void OwningCodesHandle::set(const std::string& key, Span<const char*> value) {
    delegate_.set(key, value);
};
void OwningCodesHandle::set(const std::string& key, Span<const double> value) {
    delegate_.set(key, value);
};
void OwningCodesHandle::set(const std::string& key, Span<const float> value) {
    delegate_.set(key, value);
};
void OwningCodesHandle::set(const std::string& key, Span<const long> value) {
    delegate_.set(key, value);
};
void OwningCodesHandle::set(const std::string& key, Span<const unsigned char> value) {
    delegate_.set(key, value);
};
void OwningCodesHandle::forceSet(const std::string& key, Span<const double> value) {
    delegate_.set(key, value);
}
void OwningCodesHandle::forceSet(const std::string& key, Span<const float> value) {
    delegate_.set(key, value);
}

size_t OwningCodesHandle::getSize(const std::string& key) const {
    return delegate_.getSize(key);
};

Value OwningCodesHandle::get(const std::string& key) const {
    return delegate_.get(key);
}

NativeType OwningCodesHandle::getType(const std::string& key) const {
    return delegate_.getType(key);
}

long OwningCodesHandle::getLong(const std::string& key) const {
    return delegate_.getLong(key);
}
double OwningCodesHandle::getDouble(const std::string& key) const {
    return delegate_.getDouble(key);
}
std::string OwningCodesHandle::getString(const std::string& key) const {
    return delegate_.getString(key);
}

std::vector<long> OwningCodesHandle::getLongArray(const std::string& key) const {
    return delegate_.getLongArray(key);
}
std::vector<double> OwningCodesHandle::getDoubleArray(const std::string& key) const {
    return delegate_.getDoubleArray(key);
}
std::vector<float> OwningCodesHandle::getFloatArray(const std::string& key) const {
    return delegate_.getFloatArray(key);
}
std::vector<std::string> OwningCodesHandle::getStringArray(const std::string& key) const {
    return delegate_.getStringArray(key);
}

std::vector<unsigned char> OwningCodesHandle::getBytes(const std::string& key) const {
    return delegate_.getBytes(key);
}

CodesHandlePtr* OwningCodesHandle::clone() const {
    return delegate_.clone();
}
ByteArray OwningCodesHandle::copy() const {
    return delegate_.copy();
}
KeyIterator OwningCodesHandle::keys(KeyIteratorFlags flags, std::optional<Namespace> ns) const {
    return delegate_.keys(flags, ns);
}
KeyIterator OwningCodesHandle::keys(Namespace ns) const {
    return delegate_.keys(ns);
}

GeoIterator OwningCodesHandle::values() const {
    return delegate_.values();
}
CodesHandlePtr* OwningCodesHandle::raw() const {
    return delegate_.raw();
}

CodesHandleRef OwningCodesHandle::ref() const {
    return delegate_;
}


//------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
