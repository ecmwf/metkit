/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#pragma once

#include "metkit/codes/api/CodesHandleRef.h"

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/CodesTypes.h"
#include "metkit/codes/api/GeoIterator.h"
#include "metkit/codes/api/KeyIterator.h"

#include <optional>
#include <string>
#include <vector>


namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

/// Concrete implementation of CodesHandle. OwningCodesHandle is a owning container around a CodesHandlePtr* that
/// makes the C APi accessible to C++ for any CodesHandlePtr*. It will properly delete the underlying CodesHandlePtr* on
/// construction.
/// \see OwningCodesHandle
class OwningCodesHandle : public CodesHandle {
public:  // methods

    OwningCodesHandle(CodesHandlePtr*);
    virtual ~OwningCodesHandle();

    OwningCodesHandle(const OwningCodesHandle&)            = default;
    OwningCodesHandle(OwningCodesHandle&&)                 = default;
    OwningCodesHandle& operator=(const OwningCodesHandle&) = default;
    OwningCodesHandle& operator=(OwningCodesHandle&&)      = default;


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
    void set(const std::string& key, Span<const unsigned char> value) override;  /// set bytes
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

    std::vector<unsigned char> getBytes(const std::string& key) const override;

    /// Cloning the whole handle. Expected to be wrapped by the user explicitly
    [[nodiscard]]
    CodesHandlePtr* clone() const override;

    /// Copy the message into a new allocated buffer
    ByteArray copy() const override;


    /// Iterate keys on an iterator with a range based for loop
    KeyIterator keys(KeyIteratorFlags flags         = KeyIteratorFlags::AllKeys,
                     std::optional<AnyNamespace> ns = std::optional<AnyNamespace>{}) const override;
    KeyIterator keys(AnyNamespace ns) const override;

    /// Iterate values with longitude and latituted
    GeoIterator values() const override;

    /// Access the underlying handle - no ownership is transfered
    CodesHandlePtr* raw() const override;


    /// Additional methods

    CodesHandleRef ref() const;

private:

    CodesHandlePtr* handle_;
    CodesHandleRef delegate_;
};


//------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
