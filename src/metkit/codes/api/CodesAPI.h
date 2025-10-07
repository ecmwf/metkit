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

#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

/// This file contains the important parts for an eccodes c api wrapper.
/// It does not wrap the whole `eccodes.h`.
/// The most important interface is the `CodesHandle` - it use to work on specific GRIB or BUFR messages.
/// An addition to that, a few factory functions (`codesHandleFromMessage`, `codesHandleFromSample`,
/// `codesHandleFromFile`) and iterators are wrapped.
///
/// Missing components:
///  * codes_context
///  * codes_index
///  * codes_fieldset
///  * codes_values
///  * multio_handle
///  * codes_nearest
///  * GTS handling
///  * GRIBEX

namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

/// Retrieve information about the configured samples path.
/// @return The configured path to sample files.
std::string samplesPath();

/// Retrieve information about the configured definition path.
/// @return The configured path to the definition files.
std::string definitionPath();

/// Retrieve eccodes version
/// @return The eccodes version.
long apiVersion();

/// Retrieve repository information of the built.
/// @return The SHA1 of the latest commit the eccodes library has been built on.
std::string gitSha1();

/// Retrieve repository information of the built.
/// @return The branch the eccodes library has been built on.
std::string gitBranch();

/// Retrieve repository information of the built.
/// @return The built date of the eccodes library.
std::string buildDate();

/// Retrieve repository information of the built.
/// @return The package name the library has been distributed with.
std::string packageName();

//----------------------------------------------------------------------------------------------------------------------


/// Abstract interface wrapping eccodes handles.
///
/// Eccodes exposed `codes_handle*` is used to handle GRIB1, GRIB2 and BUFR data.
class CodesHandle {
    friend class KeyRange;
    friend class GeoRange;

public:

    virtual ~CodesHandle() = default;

    /// Retrieve binary size of the handled message.
    /// @return Size of the message in bytes.
    virtual size_t messageSize() const = 0;

    /// Check if a key is defined.
    /// @param key Name of the field that is checked to be defined.
    /// @return True if the field is defined.
    virtual bool isDefined(const std::string& key) const = 0;

    /// Check if a key is missing.
    /// @param key Name of the field that is checked to be missing.
    /// @return True if the field is missing.
    virtual bool isMissing(const std::string& key) const = 0;

    /// Check if a key is defined and not missing.
    /// @param key Name of the field that is checked to exist and have a value.
    /// @return True if the field exists and has a value.
    virtual bool has(const std::string& key) const = 0;

    /// Set a key to its missing value.
    /// @param key Name of the field that is supposed to be set to its missing value.
    virtual void setMissing(const std::string& key) = 0;

    /// Set scalar string value for a specific field.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Value the field is supposed to be set to.
    virtual void set(const std::string& key, const std::string& value) = 0;

    /// Set scalar double value for a specific field.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Value the field is supposed to be set to.
    virtual void set(const std::string& key, double value) = 0;

    /// Set scalar long value for a specific field.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Value the field is supposed to be set to.
    virtual void set(const std::string& key, long value) = 0;

    /// Set scalar floating point value for a specific field.
    ///
    /// Overload for other floating point types.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Value the field is supposed to be set to.
    template <typename T, std::enable_if_t<!std::is_same_v<T, double> && std::is_floating_point_v<T>, bool> = true>
    void set(const std::string& key, const T& v) {
        set(key, static_cast<double>(v));
    }

    /// Set scalar integral value for a specific field.
    ///
    /// Overload for other integral types.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Value the field is supposed to be set on.
    template <typename T, std::enable_if_t<!std::is_same_v<T, long> && std::is_integral_v<T>, bool> = true>
    void set(const std::string& key, const T& v) {
        set(key, static_cast<long>(v));
    }

    /// Set array of string through `span` as general contiguous memory container.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Contiguous array the field is supposed to be set to.
    virtual void set(const std::string& key, Span<const std::string> value) = 0;

    /// Set array of c-string through `span` as general contiguous memory container.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Contiguous array the field is supposed to be set to.
    virtual void set(const std::string& key, Span<const char*> value) = 0;

    /// Set array of double through `span` as general contiguous memory container.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Contiguous array the field is supposed to be set to.
    virtual void set(const std::string& key, Span<const double> value) = 0;

    /// Set array of float through `span` as general contiguous memory container.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Contiguous array the field is supposed to be set to.
    virtual void set(const std::string& key, Span<const float> value) = 0;

    /// Set array of long through `span` as general contiguous memory container.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Contiguous array the field is supposed to be set to.
    virtual void set(const std::string& key, Span<const long> value) = 0;

    /// Set array of uint8_t (bytes) through `span` as general contiguous memory container.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Contiguous array the field is supposed to be set to.
    virtual void set(const std::string& key, Span<const uint8_t> value) = 0;

    /// Set any type of value as sum type for a specific field.
    /// Visits the value and calls the corresponding `set` overload.
    /// A template and SFINAE is used to match exactly the type `CodesValue`,
    /// this avoids abiguous overloads through implict convertion.
    /// @param key Name of the field that is supposed to be set.
    /// @param value Value the field is supposed to be set to.
    template <typename T, std::enable_if_t<std::is_same_v<T, CodesValue>, bool> = true>
    void set(const std::string& key, const T& value) {
        std::visit([&](const auto& v) { this->set(key, v); }, value);
    }


    /// Force setting arrays of double through `span` as general contiguous memory container.
    ///
    /// Allows setting arrays that usually get computed through other keys
    /// @param key Name of the field that is supposed to be set to a specific value.
    /// @param value Value the field is supposed to be set on.
    virtual void forceSet(const std::string& key, Span<const double> value) = 0;

    /// Force setting arrays of float through `span` as general contiguous memory container
    ///
    /// Allows setting arrays that usually get computed through other keys
    /// @param key Name of the field that is supposed to be set to a specific value.
    /// @param value Value the field is supposed to be set on.
    virtual void forceSet(const std::string& key, Span<const float> value) = 0;

    /// Returns the number of elements contained for a given key
    ///
    /// Can be used to determine if a field is storing a scalar or an array.
    /// @param key Name of the field that is supposed to be inspected.
    /// @return For given scalars 1 is returned. For given arrays the size of the array is returned..
    virtual size_t size(const std::string& key) const = 0;

    /// Get the value of the key.
    ///
    /// High-level functionality:
    /// Inspection on the contained valued is performed with `type` and `size`.
    /// Then the more specific `getXXX` call is performed.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Variant of all possible value types.
    virtual CodesValue get(const std::string& key) const = 0;

    /// Get the type of the key
    /// @param key Name of the field that is supposed to be inspected.
    /// @return Enum of the native types ECCODES exposes.
    virtual NativeType type(const std::string& key) const = 0;

    /// Get the contained value for a key as long.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved value contained for the passed key.
    virtual long getLong(const std::string& key) const = 0;

    /// Get the contained value for a key as double.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved value contained for the passed key.
    virtual double getDouble(const std::string& key) const = 0;

    /// Get the contained value for a key as string.
    ///
    /// This should be possible for all key types.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved value contained for the passed key.
    virtual std::string getString(const std::string& key) const = 0;

    /// Get the contained values for a key as array of long.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved values contained for the passed key.
    virtual std::vector<long> getLongArray(const std::string& key) const = 0;

    /// Get the contained values for a key as array of double.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved values contained for the passed key.
    virtual std::vector<double> getDoubleArray(const std::string& key) const = 0;

    /// Get the contained values for a key as array of float.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved values contained for the passed key.
    virtual std::vector<float> getFloatArray(const std::string& key) const = 0;

    /// Get the contained values for a key as array of string.
    ///
    /// This should be possible for all key types.
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved values contained for the passed key.
    virtual std::vector<std::string> getStringArray(const std::string& key) const = 0;

    /// Get the contained values for a key as array of uint8_t (bytes).
    /// @param key Name of the field that is supposed to be retrieved.
    /// @return Retrieved values contained for the passed key.
    virtual std::vector<uint8_t> getBytes(const std::string& key) const = 0;

    /// Clones the underyling handle.
    /// Uses `codes_handle_clone` internally.
    /// @return Unique pointer to a cloned `CodesHandle` instance.
    virtual std::unique_ptr<CodesHandle> clone() const = 0;

    /// Copy the message into a new allocated buffer
    /// @param data Pointer to an allocated array
    /// @param size Size of the allocated array.
    ///        Should be containing at least the size returned by `messageSize`.
    virtual void copyInto(uint8_t* data, size_t size) const = 0;


    /// Iterate keys in a GRIB2 or BUFR handle.
    ///
    /// @param flags Iterator flags to filter keys
    /// @param ns A specific namespace that is iterated (e.g. "mars" or "ls").
    ///           Namespaces are defined within definition files,
    ///           that means custom namespaces can be provided by users.
    ///           Common namespaces that are available by the default definition files
    ///           are listed in the namespace `codes::namespaces`
    /// @return A Range that can be iterated on with a range-based for-loop.
    /// @see KeyIterator
    virtual KeyRange keys(KeyIteratorFlags flags      = KeyIteratorFlags::AllKeys,
                          std::optional<Namespace> ns = std::optional<Namespace>{}) const = 0;

    /// Iterate keys in a GRIB2 or BUFR handle.
    ///
    /// @param ns A specific namespace that is iterated (e.g. "mars" or "ls").
    ///           Namespaces are defined within definition files,
    ///           that means custom namespaces can be provided by users.
    ///           Common namespaces that are available by the default definition files
    ///           are listed in the namespace `codes::namespaces`
    /// @return A Range that can be iterated on with a range-based for-loop.
    /// @see KeyIterator
    virtual KeyRange keys(Namespace ns) const = 0;

    /// Iterate values with longitude and latituted.
    ///
    /// @return A Range that can be iterated on with a range-based for-loop.
    ///         The iterated value contains a type `GeoData`
    ///         with members `longitude`, `latitude` and `value`.
    /// @see GeoIterator
    virtual GeoRange values() const = 0;

    /// Release the underlying `codes_handle*`.
    ///
    /// After calling `release` the instance of this type is in an inoperable state.
    /// Used to pass ownership out of C++ (e.g. python).
    ///
    /// Example use:
    /// ```cpp
    ///     auto rawHandle = reinterpret_cast<codes_handle*>(handl->release());
    /// ```
    ///
    /// @return Pointer to `codes_handle*`
    [[nodiscard]]
    virtual void* release() = 0;
};

/// Create a new `CodesHandle` from a byte array.
///
/// ECCODES does not copy the array until a modification is made.
/// The user needs to maintain the lifetime of the passed array.
/// @param data Array to an contiguous array of bytes.
///             Lifetime of the array need to be maintained till the first change is made on the handle.
/// @return Instance of a `CodesHandle` wrapped in a `unique_ptr`.
/// @see codesHandleFromMessageCopy
std::unique_ptr<CodesHandle> codesHandleFromMessage(Span<const uint8_t> data);

/// Create a new `CodesHandle` from a byte array.
///
/// ECCODES copies the array. No lifetime requirements exist on `data`.
/// @param data Array to an contiguous array of bytes the message is created from.
/// @return Instance of a `CodesHandle` wrapped in a `unique_ptr`.
/// @see codesHandleFromMessage
std::unique_ptr<CodesHandle> codesHandleFromMessageCopy(Span<const uint8_t> data);

/// Create a new `CodesHandle` from a sample existing in the configured samples path.
///
/// ECCODES does not copy the array until a modification is made.
/// The user needs to maintain the lifetime of the passed array.
/// @param sample Name of the sample in the sample path. Usually without the ".tmpl" suffix, e.g. "GRIB2".
/// @param product The intented type of handle that is supposed to be loaded (BUFR or GRIB).
///                Does not need to be specified.
/// @return Instance of a `CodesHandle` wrapped in a `unique_ptr`.
/// @see samplesPath
std::unique_ptr<CodesHandle> codesHandleFromSample(const std::string& sampleName,
                                                   std::optional<Product> product = std::optional<Product>{});

/// Create a new `CodesHandle` from a file.
///
/// ECCODES does not copy the array until a modification is made.
/// The user needs to maintain the lifetime of the passed array.
/// @param file Pointer to a implementation dependent file handle containing a BUFR or GRIB message.
/// @param product The intented type of handle that is supposed to be loaded (BUFR or GRIB).
/// @return Instance of a `CodesHandle` wrapped in a `unique_ptr`.
std::unique_ptr<CodesHandle> codesHandleFromFile(const std::string& fpath, Product);


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
