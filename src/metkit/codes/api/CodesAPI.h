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

#include "metkit/codes/api/CodesTypes.h"
#include "metkit/codes/api/GeoIterator.h"
#include "metkit/codes/api/KeyIterator.h"

#include <optional>
#include <string>
#include <vector>

/// This file contains the important parts for an eccodes c api wrapper.
/// It does not wrap the whole `eccodes.h`.
/// The most important interface is the `CodesHandle` - it use to work on specific GRIB or BUFR messages.
/// An addition to that a few factory functions (`newFromMessage`, `newFromSample`, `newFromFile`) and iterators are
/// wrapped.
///
/// Missing:
///  * codes_context
///  * codes_index
///  * codes_fieldset
///  * codes_values
///  * multio_handle
///  * codes_nearest
///  * GTS handling
///  * GRIBEX
///

namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

std::string samplesPath();

std::string definitionPath();

long getAPIVersion();

std::string getGitSha1();

std::string getGitBranch();

std::string getBuildDate();

std::string getPackageName();

/// High-level call with summed up information on api version, git sha1, branch, build date and package name
std::string info();

//----------------------------------------------------------------------------------------------------------------------


[[nodiscard]]
CodesHandlePtr* newFromMessage(Span<const unsigned char> data);

[[nodiscard]]
CodesHandlePtr* newFromSample(std::string sampleName, std::optional<Product> product = std::optional<Product>{});

[[nodiscard]]
CodesHandlePtr* newFromFile(FILE*, Product);


/// Abstract interface wrapping eccodes C API for handling grib messages
class CodesHandle {
    friend class KeyIterator;
    friend class GeoIterator;

public:  // methods

    virtual ~CodesHandle() {};


    /// Returns size of the message (number of bytes)
    virtual size_t messageSize() const = 0;

    /// Check if a key is defined
    virtual bool isDefined(const std::string& key) const = 0;

    /// Check if a key is missing
    virtual bool isMissing(const std::string& key) const = 0;

    /// Check if a key is defined and not missing
    virtual bool has(const std::string& key) const = 0;

    /// Set a key to its missing value
    virtual void setMissing(const std::string& key) = 0;

    /// Set scalars
    virtual void set(const std::string& key, const std::string& value) = 0;
    virtual void set(const std::string& key, double value)             = 0;
    virtual void set(const std::string& key, long value)               = 0;

    /// Set arrays
    virtual void set(const std::string& key, Span<const std::string> value)   = 0;  /// set string array
    virtual void set(const std::string& key, Span<const char*> value)         = 0;  /// set string array
    virtual void set(const std::string& key, Span<const double> value)        = 0;
    virtual void set(const std::string& key, Span<const float> value)         = 0;
    virtual void set(const std::string& key, Span<const long> value)          = 0;
    virtual void set(const std::string& key, Span<const unsigned char> value) = 0;  /// set bytes
    virtual void forceSet(const std::string& key, Span<const double> value)   = 0;
    virtual void forceSet(const std::string& key, Span<const float> value)    = 0;

    /// Returns size of the value contained for a given key (can be used to determine if an array is contained)
    virtual size_t getSize(const std::string& key) const = 0;

    /// Get the value of the key (high-level functionality)
    virtual Value get(const std::string& key) const = 0;

    /// Get the type of the key
    virtual NativeType getType(const std::string& key) const = 0;

    /// Explicit getters
    virtual long getLong(const std::string& key) const          = 0;
    virtual double getDouble(const std::string& key) const      = 0;
    virtual std::string getString(const std::string& key) const = 0;

    virtual std::vector<long> getLongArray(const std::string& key) const          = 0;
    virtual std::vector<double> getDoubleArray(const std::string& key) const      = 0;
    virtual std::vector<float> getFloatArray(const std::string& key) const        = 0;
    virtual std::vector<std::string> getStringArray(const std::string& key) const = 0;

    virtual std::vector<unsigned char> getBytes(const std::string& key) const = 0;

    /// Cloning the whole handle. Expected to be wrapped by the user explicitly
    [[nodiscard]]
    virtual CodesHandlePtr* clone() const = 0;

    /// Copy the message into a new allocated buffer
    virtual ByteArray copy() const = 0;


    /// Iterate keys on an iterator with a range based for loop
    virtual KeyIterator keys(KeyIteratorFlags flags      = KeyIteratorFlags::AllKeys,
                             std::optional<Namespace> ns = std::optional<Namespace>{}) const = 0;
    virtual KeyIterator keys(Namespace ns) const                                             = 0;

    /// Iterate values with longitude and latituted
    virtual GeoIterator values() const = 0;

    /// Access the underlying handle - no ownership is transfered
    virtual CodesHandlePtr* raw() const = 0;
};


//------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
