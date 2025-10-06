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

#include <sstream>

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

/// Concrete implementation of CodesHandle. OwningCodesHandle is a owning container around a codes_handle* that
/// makes the C APi accessible to C++ for any codes_handle*. It will properly delete the underlying codes_handle* on
/// construction.
class OwningCodesHandle : public CodesHandle {
public:  // methods

    OwningCodesHandle(std::unique_ptr<codes_handle> handle) : handle_{std::move(handle)} {};
    virtual ~OwningCodesHandle() {};

    OwningCodesHandle(OwningCodesHandle&&)            = default;
    OwningCodesHandle& operator=(OwningCodesHandle&&) = default;


    /// Overriding virtual methods

    /// Returns size of the message (number of bytes)
    size_t messageSize() const override;

    /// Check if a key is defined
    bool isDefined(const std::string& key) const override;

    /// Check if a key is missing
    bool isMissing(const std::string& key) const override;

    /// Check if a key is defined and not missing
    bool has(const std::string& key) const override;

    /// Set a key to its missing value
    void setMissing(const std::string& key) override;

    /// Set scalars
    void set(const std::string& key, const std::string& value) override;
    void set(const std::string& key, double value) override;
    void set(const std::string& key, long value) override;

    /// Set arrays
    void set(const std::string& key, Span<const std::string> value) override;  /// set string array
    void set(const std::string& key, Span<const char*> value) override;        /// set string array
    void set(const std::string& key, Span<const double> value) override;
    void set(const std::string& key, Span<const float> value) override;
    void set(const std::string& key, Span<const long> value) override;
    void set(const std::string& key, Span<const std::uint8_t> value) override;  /// set bytes
    void forceSet(const std::string& key, Span<const double> value) override;
    void forceSet(const std::string& key, Span<const float> value) override;

    /// Returns size of the value contained for a given key (can be used to determine if an array is contained)
    size_t getSize(const std::string& key) const override;

    /// Get the value of the key
    Value get(const std::string& key) const override;

    /// Get the type of the key
    NativeType getType(const std::string& key) const override;

    /// Explicit getters
    long getLong(const std::string& key) const override;
    double getDouble(const std::string& key) const override;
    std::string getString(const std::string& key) const override;

    std::vector<long> getLongArray(const std::string& key) const override;
    std::vector<double> getDoubleArray(const std::string& key) const override;
    std::vector<float> getFloatArray(const std::string& key) const override;
    std::vector<std::string> getStringArray(const std::string& key) const override;

    std::vector<std::uint8_t> getBytes(const std::string& key) const override;

    /// Cloning the whole handle. Expected to be wrapped by the user explicitly
    [[nodiscard]]
    std::unique_ptr<CodesHandle> clone() const override;

    /// Copy the message into a new allocated buffer
    /// \param data Pointer to an allocated array
    /// \param size Size of the allocated array
    void copyInto(std::uint8_t* data, size_t size) const override;


    /// Iterate keys on an iterator with a range based for loop
    KeyRange keys(KeyIteratorFlags flags      = KeyIteratorFlags::AllKeys,
                  std::optional<Namespace> ns = std::optional<Namespace>{}) const override;
    KeyRange keys(Namespace ns) const override;

    /// Iterate values with longitude and latituted
    GeoRange values() const override;

    /// Access and release the raw `codes_handle*` - used to pass ownership out of C++ (e.g. python)
    void* release() override { return handle_.release(); };

private:

    std::unique_ptr<codes_handle> handle_;
};


//----------------------------------------------------------------------------------------------------------------------
// Method implementation
//----------------------------------------------------------------------------------------------------------------------

size_t OwningCodesHandle::messageSize() const {
    size_t size;
    throwOnError(codes_get_message_size(handle_.get(), &size), Here(), "CodesHandle::messageSize()");
    return size;
}

bool OwningCodesHandle::isDefined(const std::string& key) const {
    return codes_is_defined(handle_.get(), key.c_str()) == 1;
}

bool OwningCodesHandle::isMissing(const std::string& key) const {
    int err  = 0;
    bool res = codes_is_missing(handle_.get(), key.c_str(), &err) == 1;
    throwOnError(err, Here(), "CodesHandle::isMissing()");
    return res;
}

bool OwningCodesHandle::has(const std::string& key) const {
    return isDefined(key) && !isMissing(key);
}

/// Set a key to its missing value
void OwningCodesHandle::setMissing(const std::string& key) {
    throwOnError(codes_set_missing(handle_.get(), key.c_str()), Here(), "CodesHandle::setMissing()");
}

/// Set scalars
void OwningCodesHandle::set(const std::string& key, const std::string& value) {
    size_t size = value.size();
    throwOnError(codes_set_string(handle_.get(), key.c_str(), value.c_str(), &size), Here(),
                 "CodesHandle::set(string, string)");
}
void OwningCodesHandle::set(const std::string& key, double value) {
    throwOnError(codes_set_double(handle_.get(), key.c_str(), value), Here(), "CodesHandle::set(string, double)");
}
void OwningCodesHandle::set(const std::string& key, long value) {
    throwOnError(codes_set_long(handle_.get(), key.c_str(), value), Here(), "CodesHandle::set(string, long)");
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
    throwOnError(
        codes_set_string_array(handle_.get(), key.c_str(), const_cast<const char**>(value.data()), value.size()),
        Here(), "CodesHandle::set(string, span<const char*>)");
}  /// set string array
void OwningCodesHandle::set(const std::string& key, Span<const double> value) {
    throwOnError(codes_set_double_array(handle_.get(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::set(string, span<const double>)");
}
void OwningCodesHandle::set(const std::string& key, Span<const float> value) {
    throwOnError(codes_set_float_array(handle_.get(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::set(string, span<const float>)");
}
void OwningCodesHandle::set(const std::string& key, Span<const long> value) {
    throwOnError(codes_set_long_array(handle_.get(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::set(string, span<const long>)");
}
void OwningCodesHandle::set(const std::string& key, Span<const std::uint8_t> value) {
    size_t size = value.size();
    throwOnError(codes_set_bytes(handle_.get(), key.c_str(), value.data(), &size), Here(),
                 "CodesHandle::set(string, span<const uint8_t>)");
}
void OwningCodesHandle::forceSet(const std::string& key, Span<const double> value) {
    throwOnError(codes_set_force_double_array(handle_.get(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::forceSet(string, span<const double>)");
}
void OwningCodesHandle::forceSet(const std::string& key, Span<const float> value) {
    throwOnError(codes_set_force_float_array(handle_.get(), key.c_str(), value.data(), value.size()), Here(),
                 "CodesHandle::forceSet(string, span<const float>)");
}

size_t OwningCodesHandle::getSize(const std::string& key) const {
    size_t size;
    throwOnError(codes_get_size(handle_.get(), key.c_str(), &size), Here(), "CodesHandle::getSize(string)");
    return size;
}

/// Get the value of the key
Value OwningCodesHandle::get(const std::string& key) const {
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
NativeType OwningCodesHandle::getType(const std::string& key) const {
    int type;
    throwOnError(codes_get_native_type(handle_.get(), key.c_str(), &type), Here(), "CodesHandle::getType(string)");

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
    throwOnError(codes_get_long(handle_.get(), key.c_str(), &value), Here(), "CodesHandle::getLong(string)");
    return value;
}
double OwningCodesHandle::getDouble(const std::string& key) const {
    double value;
    throwOnError(codes_get_double(handle_.get(), key.c_str(), &value), Here(), "CodesHandle::getDouble(string)");
    return value;
}
std::string OwningCodesHandle::getString(const std::string& key) const {
    std::string ret;
    std::size_t keylen = 1024;
    ret.resize(keylen);
    throwOnError(codes_get_string(handle_.get(), key.c_str(), ret.data(), &keylen), Here(),
                 "CodesHandle::getString(string)");
    ret.resize(strlen(ret.c_str()));
    return ret;
}

std::vector<long> OwningCodesHandle::getLongArray(const std::string& key) const {
    std::vector<long> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    throwOnError(codes_get_long_array(handle_.get(), key.c_str(), ret.data(), &size), Here(),
                 "CodesHandle::getLongArray(string)");
    ret.resize(size);
    return ret;
}
std::vector<double> OwningCodesHandle::getDoubleArray(const std::string& key) const {
    std::vector<double> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    throwOnError(codes_get_double_array(handle_.get(), key.c_str(), ret.data(), &size), Here(),
                 "CodesHandle::getDoubleArray(string)");
    ret.resize(size);
    return ret;
}
std::vector<float> OwningCodesHandle::getFloatArray(const std::string& key) const {
    std::vector<float> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    throwOnError(codes_get_float_array(handle_.get(), key.c_str(), ret.data(), &size), Here(),
                 "CodesHandle::getFloatArray(string)");
    ret.resize(size);
    return ret;
}
std::vector<std::string> OwningCodesHandle::getStringArray(const std::string& key) const {
    std::vector<char*> cstrings;
    std::size_t size = getSize(key);
    cstrings.resize(size);
    throwOnError(codes_get_string_array(handle_.get(), key.c_str(), cstrings.data(), &size), Here(),
                 "CodesHandle::getStringArray(string)");
    cstrings.resize(size);

    std::vector<std::string> ret;
    ret.reserve(cstrings.size());
    for (char* cstr : cstrings) {
        ret.push_back(cstr);
    }
    return ret;
}

std::vector<std::uint8_t> OwningCodesHandle::getBytes(const std::string& key) const {
    std::vector<std::uint8_t> ret;
    std::size_t size = getSize(key);
    ret.resize(size);
    throwOnError(codes_get_bytes(handle_.get(), key.c_str(), ret.data(), &size), Here(),
                 "CodesHandle::getBytes(string)");
    ret.resize(size);
    return ret;
}

/// Cloning the whole handle. Expected to be wrapped by the user explicitly
[[nodiscard]]
std::unique_ptr<CodesHandle> OwningCodesHandle::clone() const {
    std::unique_ptr<codes_handle> ret{codes_handle_clone(handle_.get())};
    if (!ret) {
        throw CodesException("CodesHandle::clone() failed", Here());
    }
    return std::make_unique<OwningCodesHandle>(std::move(ret));
}

/// Copy the message into a new allocated buffer
void OwningCodesHandle::copyInto(std::uint8_t* data, size_t size) const {
    std::size_t s = size;
    throwOnError(codes_get_message_copy(handle_.get(), data, &s), Here(), "CodesHandle::copy(uint8_t*, size_t*)");
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
        std::size_t size = refHandle_.get().getSize(key);
        ret.resize(size);
        throwOnError(codes_keys_iterator_get_long(it_.get(), ret.data(), &size), Here(), "KeyIterator::getLongArray()");
        ret.resize(size);
        return ret;
    }
    std::vector<double> getDoubleArray() const override {
        std::vector<double> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().getSize(key);
        ret.resize(size);
        throwOnError(codes_keys_iterator_get_double(it_.get(), ret.data(), &size), Here(),
                     "KeyIterator::getDoubleArray");
        ret.resize(size);
        return ret;
    }
    std::vector<float> getFloatArray() const override {
        std::vector<float> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().getSize(key);
        ret.resize(size);
        throwOnError(codes_keys_iterator_get_float(it_.get(), ret.data(), &size), Here(), "KeyIterator::getFloatArray");
        ret.resize(size);
        return ret;
    }
    std::vector<std::string> getStringArray() const override {
        // There is no other option to get string array
        return refHandle_.get().getStringArray(name());
    }

    std::vector<std::uint8_t> getBytes() const override {
        std::vector<std::uint8_t> ret;
        std::string key  = name();
        std::size_t size = refHandle_.get().getSize(key);
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
                                                         handle_.get(), mapFlags(flags), ns ? ns->c_str() : NULL)))};
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
        *this, std::unique_ptr<codes_iterator>(codes_grib_iterator_new(handle_.get(), 0, &err)))};
    throwOnError(err, Here(), "CodesHandle::values()");
    return res;
};

}  // namespace


//----------------------------------------------------------------------------------------------------------------------


std::string samplesPath() {
    return codes_samples_path(NULL);
}

std::string definitionPath() {
    return codes_definition_path(NULL);
}

long getAPIVersion() {
    return codes_get_api_version();
}

std::string getGitSha1() {
    return codes_get_git_sha1();
}

std::string getGitBranch() {
    return codes_get_git_branch();
}

std::string getBuildDate() {
    return codes_get_build_date();
}

std::string getPackageName() {
    return codes_get_package_name();
}


std::string info() {
    std::ostringstream oss;
    oss << "eccodes{";
    oss << "api-version: " << getAPIVersion();
    oss << ", git-sha1: " << getGitSha1();
    oss << ", git-branch: " << getGitBranch();
    oss << ", build-date: " << getBuildDate();
    oss << ", package-name: " << getPackageName();
    oss << "}";
    return oss.str();
}


//----------------------------------------------------------------------------------------------------------------------

[[nodiscard]]
std::unique_ptr<CodesHandle> newFromMessage(Span<const std::uint8_t> data) {
    return std::make_unique<OwningCodesHandle>(std::unique_ptr<codes_handle>(
        codes_handle_new_from_message(NULL, static_cast<const void*>(data.data()), data.size())));
}

[[nodiscard]]
std::unique_ptr<CodesHandle> newFromMessageCopy(Span<const std::uint8_t> data) {
    return std::make_unique<OwningCodesHandle>(std::unique_ptr<codes_handle>(
        codes_handle_new_from_message_copy(NULL, static_cast<const void*>(data.data()), data.size())));
}

[[nodiscard]]
std::unique_ptr<CodesHandle> newFromSample(const std::string& sampleName, std::optional<Product> product) {
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

[[nodiscard]]
std::unique_ptr<CodesHandle> newFromFile(FILE* file, Product product) {
    int err = 0;
    std::unique_ptr<codes_handle> ret;
    switch (product) {
        case Product::GRIB:
            ret = std::unique_ptr<codes_handle>(codes_grib_handle_new_from_file(NULL, file, &err));
            break;
        case Product::BUFR:
            ret = std::unique_ptr<codes_handle>(codes_bufr_handle_new_from_file(NULL, file, &err));
            break;
    };
    throwOnError(err, Here(), "newFromFile(FILE*, Product): ");
    if (!ret) {
        throw CodesException("codes_handle_new_from_file returned NULL without an additional error");
    }

    return std::make_unique<OwningCodesHandle>(std::move(ret));
}


}  // namespace metkit::codes
