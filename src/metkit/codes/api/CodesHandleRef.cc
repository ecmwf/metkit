/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/api/CodesHandleRef.h"
#include <variant>

#include "eccodes.h"
#include "metkit/codes/api/CodesTypes.h"

namespace metkit::codes {

namespace {
void checkCodes(int code) {
    if (code != 0) {
        std::string msg(codes_get_error_message(code));
        throw CodesException(msg);
    }
};

CodesHandlePtr* castFromCodes(codes_handle* ptr) {
    return static_cast<CodesHandlePtr*>(static_cast<void*>(ptr));
}

codes_handle* castToCodes(CodesHandlePtr* ptr) {
    return static_cast<codes_handle*>(static_cast<void*>(ptr));
}
}  // namespace


CodesHandleRef::CodesHandleRef(CodesHandlePtr* handle) : handle_(handle) {};


size_t CodesHandleRef::messageSize() const {
    size_t size;
    checkCodes(codes_get_message_size(castToCodes(handle_), &size));
    return size;
}

bool CodesHandleRef::isDefined(const std::string& key) const {
    return codes_is_defined(castToCodes(handle_), key.c_str()) == 1;
}

bool CodesHandleRef::isMissing(const std::string& key) const {
    int err  = 0;
    bool res = codes_is_missing(castToCodes(handle_), key.c_str(), &err) == 1;
    checkCodes(err);
    return res;
}

bool CodesHandleRef::has(const std::string& key) const {
    return isDefined(key) && !isMissing(key);
}

/// Set a key to its missing value
void CodesHandleRef::setMissing(const std::string& key) {
    checkCodes(codes_set_missing(castToCodes(handle_), key.c_str()));
}

/// Set scalars
void CodesHandleRef::set(const std::string& key, const std::string& value) {
    size_t size = value.size();
    checkCodes(codes_set_string(castToCodes(handle_), key.c_str(), value.c_str(), &size));
}
void CodesHandleRef::set(const std::string& key, double value) {
    checkCodes(codes_set_double(castToCodes(handle_), key.c_str(), value));
}
void CodesHandleRef::set(const std::string& key, long value) {
    checkCodes(codes_set_long(castToCodes(handle_), key.c_str(), value));
}

/// Set arrays
void CodesHandleRef::set(const std::string& key, Span<const std::string> value) {
    std::vector<const char*> out;
    out.reserve(value.size());
    for (auto it = value.data(); it != value.data() + value.size(); ++it) {
        out.push_back(it->c_str());
    }
    set(key, out);
}
void CodesHandleRef::set(const std::string& key, Span<const char*> value) {
    checkCodes(codes_set_string_array(castToCodes(handle_), key.c_str(), const_cast<const char**>(value.data()),
                                      value.size()));
}  /// set string array
void CodesHandleRef::set(const std::string& key, Span<const double> value) {
    checkCodes(codes_set_double_array(castToCodes(handle_), key.c_str(), value.data(), value.size()));
}
void CodesHandleRef::set(const std::string& key, Span<const float> value) {
    checkCodes(codes_set_float_array(castToCodes(handle_), key.c_str(), value.data(), value.size()));
}
void CodesHandleRef::set(const std::string& key, Span<const long> value) {
    checkCodes(codes_set_long_array(castToCodes(handle_), key.c_str(), value.data(), value.size()));
}
void CodesHandleRef::set(const std::string& key, Span<const unsigned char> value) {
    size_t size = value.size();
    checkCodes(codes_set_bytes(castToCodes(handle_), key.c_str(), value.data(), &size));
}
void CodesHandleRef::forceSet(const std::string& key, Span<const double> value) {
    checkCodes(codes_set_force_double_array(castToCodes(handle_), key.c_str(), value.data(), value.size()));
}
void CodesHandleRef::forceSet(const std::string& key, Span<const float> value) {
    checkCodes(codes_set_force_float_array(castToCodes(handle_), key.c_str(), value.data(), value.size()));
}

size_t CodesHandleRef::getSize(const std::string& key) const {
    size_t size;
    checkCodes(codes_get_size(castToCodes(handle_), key.c_str(), &size));
    return size;
}

/// Get the value of the key
Value CodesHandleRef::get(const std::string& key) const {
    NativeType type = getType(key);
    bool isArray    = getSize(key) > 1;

    switch (type) {
        case NativeType::Long:
            if (isArray) {
                return getLongArray(key);
            }
            else {
                return getLong(key);
            }
        case NativeType::Double:
            if (isArray) {
                return getDoubleArray(key);
            }
            else {
                return getDouble(key);
            }
        case NativeType::String:
            if (isArray) {
                return getStringArray(key);
            }
            else {
                return getString(key);
            }
        case NativeType::Bytes:
            return getBytes(key);
        case NativeType::Undefined:
            throw CodesException(std::string("CodesHandle::get(") + key + std::string("): Native type is UNDEFINED"));
        case NativeType::Section:
        case NativeType::Label:
        case NativeType::Missing:
        default:
            return getString(key);
    }
}

/// Get the type of the key
NativeType CodesHandleRef::getType(const std::string& key) const {
    int type;
    checkCodes(codes_get_native_type(castToCodes(handle_), key.c_str(), &type));

    switch (type) {
        case CODES_TYPE_LONG:
            return NativeType::Long;
        case CODES_TYPE_DOUBLE:
            return NativeType::Double;
        case CODES_TYPE_STRING:
            return NativeType::String;
        case CODES_TYPE_BYTES:
            return NativeType::Bytes;
        case CODES_TYPE_SECTION:
            return NativeType::Section;
        case CODES_TYPE_LABEL:
            return NativeType::Label;
        case CODES_TYPE_MISSING:
            return NativeType::Missing;
        default:
            return NativeType::Undefined;
    }
}

/// Explicit getters
long CodesHandleRef::getLong(const std::string& key) const {
    long value;
    checkCodes(codes_get_long(castToCodes(handle_), key.c_str(), &value));
    return value;
}
double CodesHandleRef::getDouble(const std::string& key) const {
    double value;
    checkCodes(codes_get_double(castToCodes(handle_), key.c_str(), &value));
    return value;
}
std::string CodesHandleRef::getString(const std::string& key) const {
    std::string ret;
    std::size_t keylen = 1024;
    ret.resize(keylen);
    checkCodes(codes_get_string(castToCodes(handle_), key.c_str(), ret.data(), &keylen));
    ret.resize(strlen(ret.c_str()));
    return ret;
}

std::vector<long> CodesHandleRef::getLongArray(const std::string& key) const {
    std::vector<long> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    checkCodes(codes_get_long_array(castToCodes(handle_), key.c_str(), ret.data(), &size));
    ret.resize(size);
    return ret;
}
std::vector<double> CodesHandleRef::getDoubleArray(const std::string& key) const {
    std::vector<double> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    checkCodes(codes_get_double_array(castToCodes(handle_), key.c_str(), ret.data(), &size));
    ret.resize(size);
    return ret;
}
std::vector<float> CodesHandleRef::getFloatArray(const std::string& key) const {
    std::vector<float> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    checkCodes(codes_get_float_array(castToCodes(handle_), key.c_str(), ret.data(), &size));
    ret.resize(size);
    return ret;
}
std::vector<std::string> CodesHandleRef::getStringArray(const std::string& key) const {
    std::vector<char*> cstrings;
    std::size_t size = getSize(key);
    cstrings.resize(size);
    checkCodes(codes_get_string_array(castToCodes(handle_), key.c_str(), cstrings.data(), &size));
    cstrings.resize(size);

    std::vector<std::string> ret;
    ret.reserve(cstrings.size());
    for (char* cstr : cstrings) {
        ret.push_back(cstr);
    }
    return ret;
}

std::vector<unsigned char> CodesHandleRef::getBytes(const std::string& key) const {
    std::vector<unsigned char> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    checkCodes(codes_get_bytes(castToCodes(handle_), key.c_str(), ret.data(), &size));
    ret.resize(size);
    return ret;
}

/// Cloning the whole handle. Expected to be wrapped by the user explicitly
[[nodiscard]]
CodesHandlePtr* CodesHandleRef::clone() const {
    auto ret = castFromCodes(codes_handle_clone(castToCodes(handle_)));
    if (ret == NULL) {
        throw CodesException("Cloning failed");
    }
    return ret;
}

/// Copy the message into a new allocated buffer
ByteArray CodesHandleRef::copy() const {
    auto res    = ByteArray::makeForOverwrite(messageSize());
    size_t size = res.size();
    checkCodes(codes_get_message_copy(castToCodes(handle_), res.data(), &size));
    return res;
}


class ConcreteIteratedKey : public IteratedKey {
public:  // methods

    ConcreteIteratedKey(const CodesHandleRef& handle, codes_keys_iterator* it) :
        refHandle_{std::ref(handle)}, it_{it}, isValid_{false} {
        next();
    }

    virtual ~ConcreteIteratedKey() {
        if (it_ != NULL) {
            checkCodes(codes_keys_iterator_delete(it_));
            it_ = NULL;
        }
    }

    std::string name() const override { return codes_keys_iterator_get_name(it_); };

    /// Get the value of the key
    Value get() const override {
        std::string key = name();
        NativeType type = refHandle_.get().getType(key);
        bool isArray    = refHandle_.get().getSize(key) > 1;

        switch (type) {
            case NativeType::Long:
                if (isArray) {
                    return getLongArray();
                }
                else {
                    return getLong();
                }
            case NativeType::Double:
                if (isArray) {
                    return getDoubleArray();
                }
                else {
                    return getDouble();
                }
            case NativeType::String:
                if (isArray) {
                    return getStringArray();
                }
                else {
                    return getString();
                }
            case NativeType::Bytes:
                return getBytes();
            case NativeType::Undefined:
                throw CodesException(std::string("IteratedKey::get(") + key +
                                     std::string("): Native type is UNDEFINED"));
            case NativeType::Section:
            case NativeType::Label:
            case NativeType::Missing:
            default:
                return getString();
        }
    };

    /// Get the type of the key
    NativeType getType() const override { return refHandle_.get().getType(name()); };

    /// Explicit getters
    long getLong() const override {
        long value;
        size_t size = 1;
        checkCodes(codes_keys_iterator_get_long(it_, &value, &size));
        return value;
    }
    double getDouble() const override {
        double value;
        size_t size = 1;
        checkCodes(codes_keys_iterator_get_double(it_, &value, &size));
        return value;
    }
    float getFloat() const override {
        float value;
        size_t size = 1;
        checkCodes(codes_keys_iterator_get_float(it_, &value, &size));
        return value;
    }
    std::string getString() const override {
        std::string ret;
        std::size_t keylen = 1024;
        ret.resize(keylen);
        checkCodes(codes_keys_iterator_get_string(it_, ret.data(), &keylen));
        ret.resize(strlen(ret.c_str()));
        return ret;
    }

    std::vector<long> getLongArray() const override {
        std::vector<long> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().getSize(key);
        ret.resize(size);
        checkCodes(codes_keys_iterator_get_long(it_, ret.data(), &size));
        ret.resize(size);
        return ret;
    }
    std::vector<double> getDoubleArray() const override {
        std::vector<double> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().getSize(key);
        ret.resize(size);
        checkCodes(codes_keys_iterator_get_double(it_, ret.data(), &size));
        ret.resize(size);
        return ret;
    }
    std::vector<float> getFloatArray() const override {
        std::vector<float> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().getSize(key);
        ret.resize(size);
        checkCodes(codes_keys_iterator_get_float(it_, ret.data(), &size));
        ret.resize(size);
        return ret;
    }
    std::vector<std::string> getStringArray() const override {
        // There is no other option to get string array
        return refHandle_.get().getStringArray(name());
    }

    std::vector<unsigned char> getBytes() const override {
        std::vector<unsigned char> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().getSize(key);
        ret.resize(size);
        checkCodes(codes_keys_iterator_get_bytes(it_, ret.data(), &size));
        ret.resize(size);
        return ret;
    }

protected:

    void next() override { isValid_ = codes_keys_iterator_next(it_) > 0; }
    bool isValid() const override { return isValid_; }

private:

    std::reference_wrapper<const CodesHandleRef> refHandle_;
    codes_keys_iterator* it_;
    bool isValid_;
};

unsigned long mapFlags(KeyIteratorFlags flags) {
    unsigned long res = CODES_KEYS_ITERATOR_ALL_KEYS;

    if (hasFlag(flags, KeyIteratorFlags::SkipReadOnly)) {
        res |= CODES_KEYS_ITERATOR_SKIP_READ_ONLY;
    }
    if (hasFlag(flags, KeyIteratorFlags::SkipOptional)) {
        res |= CODES_KEYS_ITERATOR_SKIP_OPTIONAL;
    }
    if (hasFlag(flags, KeyIteratorFlags::SkipEditionSpecific)) {
        res |= CODES_KEYS_ITERATOR_SKIP_EDITION_SPECIFIC;
    }
    if (hasFlag(flags, KeyIteratorFlags::SkipCoded)) {
        res |= CODES_KEYS_ITERATOR_SKIP_CODED;
    }
    if (hasFlag(flags, KeyIteratorFlags::SkipComputed)) {
        res |= CODES_KEYS_ITERATOR_SKIP_COMPUTED;
    }
    if (hasFlag(flags, KeyIteratorFlags::SkipDuplicates)) {
        res |= CODES_KEYS_ITERATOR_SKIP_DUPLICATES;
    }
    if (hasFlag(flags, KeyIteratorFlags::SkipFunction)) {
        res |= CODES_KEYS_ITERATOR_SKIP_FUNCTION;
    }

    return res;
}

/// Iterate keys on an iterator with a range based for loop
KeyIterator CodesHandleRef::keys(KeyIteratorFlags flags, std::optional<Namespace> ns) const {
    return KeyIterator{std::make_unique<ConcreteIteratedKey>(
        *this,
        codes_keys_iterator_new(castToCodes(handle_), mapFlags(flags), ns ? ns->c_str() : NULL))};
};

KeyIterator CodesHandleRef::keys(Namespace ns) const {
    return keys(KeyIteratorFlags::AllKeys, ns);
};


class ConcreteIteratedGeoData : public IteratedGeoData {
public:  // methods

    ConcreteIteratedGeoData(const CodesHandleRef& handle, codes_iterator* it) :
        refHandle_{std::ref(handle)}, it_{it}, data_{0.0, 0.0, 0.0}, isValid_{false} {
        if (hasNext()) {
            next();
        }
    }

    virtual ~ConcreteIteratedGeoData() {
        if (it_ != NULL) {
            checkCodes(codes_grib_iterator_delete(it_));
            it_ = NULL;
        }
    }

    /// Returns the name of the key
    GeoData data() const override { return data_; };

    bool hasNext() const override { return codes_grib_iterator_has_next(it_) > 0; };


protected:

    void next() override {
        isValid_ = codes_grib_iterator_next(it_, &data_.latitude, &data_.longitude, &data_.value) > 0;
    }

    bool isValid() const override { return isValid_; }

private:

    std::reference_wrapper<const CodesHandleRef> refHandle_;
    codes_iterator* it_;
    GeoData data_;
    bool isValid_;
};

/// Iterate CodesHandleRef::values with longitude and latituted
GeoIterator CodesHandleRef::values() const {
    int err;
    GeoIterator res{
        std::make_unique<ConcreteIteratedGeoData>(*this, codes_grib_iterator_new(castToCodes(handle_), 0, &err))};
    checkCodes(err);
    return res;
};

/// Access the underlying handle - no ownership is transfered
CodesHandlePtr* CodesHandleRef::raw() const {
    return handle_;
};


//------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
