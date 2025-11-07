/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/CodesTypes.h"

#include "eckit/log/CodeLocation.h"

#include "eccodes.h"

namespace std {
template <>
struct default_delete<codes_handle> {
    void operator()(codes_handle* h) { ::codes_handle_delete(h); }
};

template <>
struct default_delete<codes_keys_iterator> {
    void operator()(codes_keys_iterator* it) { ::codes_keys_iterator_delete(it); }
};

template <>
struct default_delete<codes_iterator> {
    void operator()(codes_iterator* it) { ::codes_grib_iterator_delete(it); }
};
}  // namespace std


namespace metkit::codes {


namespace {

void throwOnError(int code, const eckit::CodeLocation& l, const char* details) {
    if (code != 0) {
        std::string msg = std::string(details) + std::string(": ") + std::string(codes_get_error_message(code));
        throw CodesException(msg, l);
    }
};

void throwOnError(int code, const eckit::CodeLocation& l, const char* details, const std::string& key) {
    if (code != 0) {
        std::string msg = std::string(details) + std::string(": ") + std::string(codes_get_error_message(code)) +
                          std::string(" for key ") + key;
        throw CodesException(msg, l);
    }
};

/// Concrete implementation of CodesHandle.
/// OwningCodesHandle is a owning container around a codes_handle* that
/// makes the C APi accessible to C++ for any codes_handle*.
/// It will properly delete the underlying codes_handle* on
/// construction.
class OwningCodesHandle : public CodesHandle {
public:

    OwningCodesHandle(std::unique_ptr<codes_handle> handle) : handle_{std::move(handle)} {};
    virtual ~OwningCodesHandle() {};

    OwningCodesHandle(OwningCodesHandle&&)            = default;
    OwningCodesHandle& operator=(OwningCodesHandle&&) = default;


    size_t messageSize() const override;
    bool isDefined(const std::string& key) const override;
    bool isMissing(const std::string& key) const override;
    bool has(const std::string& key) const override;
    void setMissing(const std::string& key) override;
    void set(const std::string& key, const std::string& value) override;
    void set(const std::string& key, double value) override;
    void set(const std::string& key, long value) override;
    void set(const std::string& key, Span<const std::string> value) override;
    void set(const std::string& key, Span<const char*> value) override;
    void set(const std::string& key, Span<const double> value) override;
    void set(const std::string& key, Span<const float> value) override;
    void set(const std::string& key, Span<const long> value) override;
    void set(const std::string& key, Span<const uint8_t> value) override;
    void forceSet(const std::string& key, Span<const double> value) override;
    void forceSet(const std::string& key, Span<const float> value) override;
    size_t size(const std::string& key) const override;
    CodesValue get(const std::string& key) const override;
    NativeType type(const std::string& key) const override;
    long getLong(const std::string& key) const override;
    double getDouble(const std::string& key) const override;
    std::string getString(const std::string& key) const override;
    std::vector<long> getLongArray(const std::string& key) const override;
    std::vector<double> getDoubleArray(const std::string& key) const override;
    std::vector<float> getFloatArray(const std::string& key) const override;
    std::vector<std::string> getStringArray(const std::string& key) const override;
    std::vector<uint8_t> getBytes(const std::string& key) const override;

    /// Clones the underyling handle.
    /// Uses `codes_handle_clone` internally.
    /// @return Unique pointer to a cloned `CodesHandle` instance.
    std::unique_ptr<CodesHandle> clone() const override;

    /// Copy the message into a new allocated buffer
    /// @param data Pointer to an allocated array
    /// @param size Size of the allocated array
    void copyInto(uint8_t* data, size_t size) const override;


    KeyRange keys(KeyIteratorFlags flags      = KeyIteratorFlags::AllKeys,
                  std::optional<Namespace> ns = std::optional<Namespace>{}) const override;
    KeyRange keys(Namespace ns) const override;

    GeoRange values() const override;

    /// Release the raw `codes_handle*` - used to pass ownership out of C++ (e.g. python)
    void* release() override { return handle_.release(); };

protected:

    codes_handle* raw() const {
        if (!handle_) {
            throw CodesException("CodesHandle has been released.", Here());
        }
        return handle_.get();
    }

private:

    std::unique_ptr<codes_handle> handle_;
};


size_t OwningCodesHandle::messageSize() const {
    size_t size;
    throwOnError(codes_get_message_size(raw(), &size), Here(), "CodesHandle::messageSize()");
    return size;
}

bool OwningCodesHandle::isDefined(const std::string& key) const {
    return codes_is_defined(raw(), key.c_str()) == 1;
}

bool OwningCodesHandle::isMissing(const std::string& key) const {
    int err  = 0;
    bool res = codes_is_missing(raw(), key.c_str(), &err) == 1;
    throwOnError(err, Here(), "CodesHandle::isMissing()", key);
    return res;
}

bool OwningCodesHandle::has(const std::string& key) const {
    return isDefined(key) && !isMissing(key);
}

/// Set a key to its missing value
void OwningCodesHandle::setMissing(const std::string& key) {
    throwOnError(codes_set_missing(raw(), key.c_str()), Here(), "CodesHandle::setMissing()", key);
}

void OwningCodesHandle::set(const std::string& key, const std::string& value) {
    size_t size = value.size();
    throwOnError(codes_set_string(raw(), key.c_str(), value.c_str(), &size), Here(), "CodesHandle::set(string, string)",
                 key);
}
void OwningCodesHandle::set(const std::string& key, double value) {
    throwOnError(codes_set_double(raw(), key.c_str(), value), Here(), "CodesHandle::set(string, double)", key);
}
void OwningCodesHandle::set(const std::string& key, long value) {
    throwOnError(codes_set_long(raw(), key.c_str(), value), Here(), "CodesHandle::set(string, long)", key);
}

/// Set arrays
void OwningCodesHandle::set(const std::string& key, Span<const std::string> value) {
    std::vector<const char*> out;
    out.reserve(value.size());
    for (auto it = value.data(); it != value.data() + value.size(); ++it) {
        out.push_back(it->c_str());
    }
    set(key, out);
}
void OwningCodesHandle::set(const std::string& key, Span<const char*> value) {
    throwOnError(codes_set_string_array(raw(), key.c_str(), const_cast<const char**>(value.data()), value.size()),
                 Here(), "CodesHandle::set(string, span<const char*>)", key);
}  /// set string array
void OwningCodesHandle::set(const std::string& key, Span<const double> value) {
    throwOnError(codes_set_double_array(raw(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::set(string, span<const double>)", key);
}
void OwningCodesHandle::set(const std::string& key, Span<const float> value) {
    throwOnError(codes_set_float_array(raw(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::set(string, span<const float>)", key);
}
void OwningCodesHandle::set(const std::string& key, Span<const long> value) {
    throwOnError(codes_set_long_array(raw(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::set(string, span<const long>)", key);
}
void OwningCodesHandle::set(const std::string& key, Span<const uint8_t> value) {
    size_t size = value.size();
    throwOnError(codes_set_bytes(raw(), key.c_str(), value.data(), &size), Here(),
                 "CodesHandle::set(string, span<const uint8_t>)", key);
}
void OwningCodesHandle::forceSet(const std::string& key, Span<const double> value) {
    throwOnError(codes_set_force_double_array(raw(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::forceSet(string, span<const double>)", key);
}
void OwningCodesHandle::forceSet(const std::string& key, Span<const float> value) {
    throwOnError(codes_set_force_float_array(raw(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::forceSet(string, span<const float>)", key);
}

size_t OwningCodesHandle::size(const std::string& key) const {
    size_t size;
    throwOnError(codes_get_size(raw(), key.c_str(), &size), Here(), "CodesHandle::size(string)", key);
    return size;
}

/// Get the value of the key
CodesValue OwningCodesHandle::get(const std::string& key) const {
    NativeType ktype = type(key);
    bool isArray     = size(key) > 1;

    switch (ktype) {
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
            throw CodesException(std::string("CodesHandle::get(") + key + std::string("): Native type is UNDEFINED"),
                                 Here());
        case NativeType::Section:
        case NativeType::Label:
        case NativeType::Missing:
        default:
            return getString(key);
    }
}

/// Get the type of the key
NativeType OwningCodesHandle::type(const std::string& key) const {
    int type;
    throwOnError(codes_get_native_type(raw(), key.c_str(), &type), Here(), "CodesHandle::type(string)", key);

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
long OwningCodesHandle::getLong(const std::string& key) const {
    long value;
    throwOnError(codes_get_long(raw(), key.c_str(), &value), Here(), "CodesHandle::getLong(string)", key);
    return value;
}
double OwningCodesHandle::getDouble(const std::string& key) const {
    double value;
    throwOnError(codes_get_double(raw(), key.c_str(), &value), Here(), "CodesHandle::getDouble(string)", key);
    return value;
}
std::string OwningCodesHandle::getString(const std::string& key) const {
    std::string ret;
    std::size_t keylen = 1024;
    ret.resize(keylen);
    throwOnError(codes_get_string(raw(), key.c_str(), ret.data(), &keylen), Here(), "CodesHandle::getString(string)",
                 key);
    ret.resize(strlen(ret.c_str()));
    return ret;
}

std::vector<long> OwningCodesHandle::getLongArray(const std::string& key) const {
    std::vector<long> ret;
    std::size_t ksize = size(key);
    ret.resize(ksize);
    throwOnError(codes_get_long_array(raw(), key.c_str(), ret.data(), &ksize), Here(),
                 "CodesHandle::getLongArray(string)", key);
    ret.resize(ksize);
    return ret;
}
std::vector<double> OwningCodesHandle::getDoubleArray(const std::string& key) const {
    std::vector<double> ret;
    std::size_t ksize = size(key);
    ret.resize(ksize);
    throwOnError(codes_get_double_array(raw(), key.c_str(), ret.data(), &ksize), Here(),
                 "CodesHandle::getDoubleArray(string)", key);
    ret.resize(ksize);
    return ret;
}
std::vector<float> OwningCodesHandle::getFloatArray(const std::string& key) const {
    std::vector<float> ret;
    std::size_t ksize = size(key);
    ret.resize(ksize);
    throwOnError(codes_get_float_array(raw(), key.c_str(), ret.data(), &ksize), Here(),
                 "CodesHandle::getFloatArray(string)", key);
    ret.resize(ksize);
    return ret;
}
std::vector<std::string> OwningCodesHandle::getStringArray(const std::string& key) const {
    std::vector<char*> cstrings;
    std::size_t ksize = size(key);
    cstrings.resize(ksize);
    throwOnError(codes_get_string_array(raw(), key.c_str(), cstrings.data(), &ksize), Here(),
                 "CodesHandle::getStringArray(string)", key);
    cstrings.resize(ksize);

    std::vector<std::string> ret;
    ret.reserve(cstrings.size());
    for (char* cstr : cstrings) {
        ret.push_back(cstr);
    }
    return ret;
}

std::vector<uint8_t> OwningCodesHandle::getBytes(const std::string& key) const {
    std::vector<uint8_t> ret;
    std::size_t ksize = size(key);
    ret.resize(ksize);
    throwOnError(codes_get_bytes(raw(), key.c_str(), ret.data(), &ksize), Here(), "CodesHandle::getBytes(string)", key);
    ret.resize(ksize);
    return ret;
}

/// Cloning the whole handle. Expected to be wrapped by the user explicitly
std::unique_ptr<CodesHandle> OwningCodesHandle::clone() const {
    std::unique_ptr<codes_handle> ret{codes_handle_clone(raw())};
    if (!ret) {
        throw CodesException("CodesHandle::clone() failed", Here());
    }
    return std::make_unique<OwningCodesHandle>(std::move(ret));
}

/// Copy the message into a new allocated buffer
void OwningCodesHandle::copyInto(uint8_t* data, size_t size) const {
    std::size_t s = size;
    throwOnError(codes_get_message_copy(raw(), data, &s), Here(), "CodesHandle::copy(uint8_t*, size_t*)");
}


class ConcreteKeyIterator : public KeyIterator {
public:  // methods

    ConcreteKeyIterator(const OwningCodesHandle& handle, std::unique_ptr<codes_keys_iterator> it) :
        refHandle_{std::cref(handle)}, it_{std::move(it)}, isValid_{false} {
        next();
    }

    virtual ~ConcreteKeyIterator() {}

    std::string name() const override { return codes_keys_iterator_get_name(it_.get()); };

    /// Get the value of the key
    CodesValue get() const override {
        std::string key = name();
        NativeType type = refHandle_.get().type(key);
        bool isArray    = refHandle_.get().size(key) > 1;

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
    NativeType type() const override { return refHandle_.get().type(name()); };

    /// Explicit getters
    long getLong() const override {
        long value;
        size_t size = 1;
        throwOnError(codes_keys_iterator_get_long(it_.get(), &value, &size), Here(), "KeyIterator::getLong()");
        return value;
    }
    double getDouble() const override {
        double value;
        size_t size = 1;
        throwOnError(codes_keys_iterator_get_double(it_.get(), &value, &size), Here(), "KeyIterator::getDouble()");
        return value;
    }
    float getFloat() const override {
        float value;
        size_t size = 1;
        throwOnError(codes_keys_iterator_get_float(it_.get(), &value, &size), Here(), "KeyIterator::getFloat()");
        return value;
    }
    std::string getString() const override {
        std::string ret;
        std::size_t keylen = 1024;
        ret.resize(keylen);
        throwOnError(codes_keys_iterator_get_string(it_.get(), ret.data(), &keylen), Here(),
                     "KeyIterator::getString()");
        ret.resize(strlen(ret.c_str()));
        return ret;
    }

    std::vector<long> getLongArray() const override {
        std::vector<long> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().size(key);
        ret.resize(size);
        throwOnError(codes_keys_iterator_get_long(it_.get(), ret.data(), &size), Here(), "KeyIterator::getLongArray()");
        ret.resize(size);
        return ret;
    }
    std::vector<double> getDoubleArray() const override {
        std::vector<double> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().size(key);
        ret.resize(size);
        throwOnError(codes_keys_iterator_get_double(it_.get(), ret.data(), &size), Here(),
                     "KeyIterator::getDoubleArray");
        ret.resize(size);
        return ret;
    }
    std::vector<float> getFloatArray() const override {
        std::vector<float> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().size(key);
        ret.resize(size);
        throwOnError(codes_keys_iterator_get_float(it_.get(), ret.data(), &size), Here(), "KeyIterator::getFloatArray");
        ret.resize(size);
        return ret;
    }
    std::vector<std::string> getStringArray() const override {
        // There is no other option to get string array
        return refHandle_.get().getStringArray(name());
    }

    std::vector<uint8_t> getBytes() const override {
        std::vector<uint8_t> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().size(key);
        ret.resize(size);
        throwOnError(codes_keys_iterator_get_bytes(it_.get(), ret.data(), &size), Here(), "KeyIterator::getBytes()");
        ret.resize(size);
        return ret;
    }

protected:

    void next() override { isValid_ = codes_keys_iterator_next(it_.get()) > 0; }
    bool isValid() const override { return isValid_; }

private:

    std::reference_wrapper<const OwningCodesHandle> refHandle_;
    std::unique_ptr<codes_keys_iterator> it_;
    bool isValid_;
};

unsigned long mapFlags(KeyIteratorFlags flags) {
    unsigned long res = 0;

    if (hasFlag(flags, KeyIteratorFlags::AllKeys)) {
        res |= CODES_KEYS_ITERATOR_ALL_KEYS;
    }
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
KeyRange OwningCodesHandle::keys(KeyIteratorFlags flags, std::optional<Namespace> ns) const {
    return KeyRange{
        std::make_unique<ConcreteKeyIterator>(*this, std::unique_ptr<codes_keys_iterator>(codes_keys_iterator_new(
                                                         raw(), mapFlags(flags), ns ? ns->c_str() : NULL)))};
};

KeyRange OwningCodesHandle::keys(Namespace ns) const {
    return keys(KeyIteratorFlags::AllKeys, ns);
};


class ConcreteIteratedGeoData : public GeoIterator {
public:  // methods

    ConcreteIteratedGeoData(const OwningCodesHandle& handle, std::unique_ptr<codes_iterator> it) :
        refHandle_{std::ref(handle)}, it_{std::move(it)}, data_{0.0, 0.0, 0.0}, isValid_{false} {
        if (hasNext()) {
            next();
        }
    }

    virtual ~ConcreteIteratedGeoData() {}

    /// Returns the name of the key
    const GeoData& data() const override { return data_; };

    bool hasNext() const override { return codes_grib_iterator_has_next(it_.get()) > 0; };


protected:

    void next() override {
        isValid_ = codes_grib_iterator_next(it_.get(), &data_.latitude, &data_.longitude, &data_.value) > 0;
    }

    bool isValid() const override { return isValid_; }

private:

    std::reference_wrapper<const OwningCodesHandle> refHandle_;
    std::unique_ptr<codes_iterator> it_;
    GeoData data_;
    bool isValid_;
};

/// Iterate OwningCodesHandle::values with longitude and latituted
GeoRange OwningCodesHandle::values() const {
    int err;
    GeoRange res{std::make_unique<ConcreteIteratedGeoData>(
        *this, std::unique_ptr<codes_iterator>(codes_grib_iterator_new(raw(), 0, &err)))};
    throwOnError(err, Here(), "CodesHandle::values()");
    return res;
};

}  // namespace


std::string samplesPath() {
    return codes_samples_path(NULL);
}

std::string definitionPath() {
    return codes_definition_path(NULL);
}

long apiVersion() {
    return codes_get_api_version();
}

std::string gitSha1() {
    return codes_get_git_sha1();
}

std::string gitBranch() {
    return codes_get_git_branch();
}

std::string buildDate() {
    return codes_get_build_date();
}

std::string packageName() {
    return codes_get_package_name();
}


std::unique_ptr<CodesHandle> codesHandleFromMessage(Span<const uint8_t> data) {
    return std::make_unique<OwningCodesHandle>(std::unique_ptr<codes_handle>(
        codes_handle_new_from_message(NULL, static_cast<const void*>(data.data()), data.size())));
}

std::unique_ptr<CodesHandle> codesHandleFromMessageCopy(Span<const uint8_t> data) {
    return std::make_unique<OwningCodesHandle>(std::unique_ptr<codes_handle>(
        codes_handle_new_from_message_copy(NULL, static_cast<const void*>(data.data()), data.size())));
}

std::unique_ptr<CodesHandle> codesHandleFromSample(const std::string& sampleName, std::optional<Product> product) {
    if (product) {
        switch (*product) {
            case Product::GRIB:
                return std::make_unique<OwningCodesHandle>(
                    std::unique_ptr<codes_handle>(codes_grib_handle_new_from_samples(NULL, sampleName.c_str())));
            case Product::BUFR:
                return std::make_unique<OwningCodesHandle>(
                    std::unique_ptr<codes_handle>(codes_bufr_handle_new_from_samples(NULL, sampleName.c_str())));
            default:
                return std::make_unique<OwningCodesHandle>(
                    std::unique_ptr<codes_handle>(codes_handle_new_from_samples(NULL, sampleName.c_str())));
        }
    }
    return std::make_unique<OwningCodesHandle>(
        std::unique_ptr<codes_handle>(codes_handle_new_from_samples(NULL, sampleName.c_str())));
}

std::unique_ptr<CodesHandle> codesHandleFromFile(const std::string& fpath, Product product) {
    int err = 0;
    std::unique_ptr<codes_handle> ret;

    FILE* file = fopen(fpath.c_str(), "rb");  // "w" means write mode

    if (file == nullptr) {
        throw CodesException(std::string("Error opening file ") + fpath, Here());
    }

    switch (product) {
        case Product::GRIB:
            ret = std::unique_ptr<codes_handle>(codes_grib_handle_new_from_file(NULL, file, &err));
            break;
        case Product::BUFR:
            ret = std::unique_ptr<codes_handle>(codes_bufr_handle_new_from_file(NULL, file, &err));
            break;
    };
    fclose(file);
    throwOnError(err, Here(), "codesHandleFromFile(FILE*, Product): ");
    if (!ret) {
        throw CodesException("codes_handle_new_from_file returned NULL without an additional error");
    }

    return std::make_unique<OwningCodesHandle>(std::move(ret));
}


}  // namespace metkit::codes
