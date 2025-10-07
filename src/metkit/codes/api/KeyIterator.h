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

#include <memory>
#include <string>
#include <vector>


namespace metkit::codes {

/// A namepspace is defined through the definition files and represented as a string.
using Namespace = std::string;


/// Commonly used eccodes namespacess.
namespace namespaces {

/// LS namespace contains a collection of descriptive keys as used by `grib_ls`.
const Namespace ls{"ls"};

const Namespace parameter{"parameter"};
const Namespace statistics{"statistics"};
const Namespace time{"time"};
const Namespace geography{"geography"};
const Namespace vertical{"vertical"};

/// MARS namespace contains keys used for indexation
const Namespace mars{"mars"};

};  // namespace namespaces

/// Flag to control/filter key iterator.
///
/// Use `AllKeys` to disable filtering.
/// Use combination of `Skip*` to filter.
enum class KeyIteratorFlags : unsigned int {
    /// Default - Will iterate all keys in a namespace.
    AllKeys             = 0,
    SkipReadOnly        = 1 << 0,
    SkipOptional        = 1 << 1,
    SkipEditionSpecific = 1 << 2,
    SkipCoded           = 1 << 3,
    SkipComputed        = 1 << 4,
    SkipDuplicates      = 1 << 5,
    SkipFunction        = 1 << 6,
};

KeyIteratorFlags operator|(KeyIteratorFlags a, KeyIteratorFlags b);
KeyIteratorFlags operator&(KeyIteratorFlags a, KeyIteratorFlags b);
KeyIteratorFlags operator^(KeyIteratorFlags a, KeyIteratorFlags b);
KeyIteratorFlags operator~(KeyIteratorFlags a);

/// Optional helper to check if a flag is set.
bool hasFlag(KeyIteratorFlags value, KeyIteratorFlags flag);


//----------------------------------------------------------------------------------------------------------------------

class KeyRange;

/// Abstract interface wrapping C API calls on on key_iterator.
class KeyIterator {
    friend class KeyRange;

public:

    virtual ~KeyIterator() = default;

    /// Get the name of the iterated key.
    /// @return Name of the iterated key.
    virtual std::string name() const = 0;

    /// Get the value of the key.
    ///
    /// High-level functionality:
    /// Inspection on the contained valued is performed with `type` and `size`.
    /// Then the more specific `getXXX` call is performed.
    /// @return Variant of all possible value types.
    virtual CodesValue get() const = 0;

    /// Get the type of the iterated key.
    /// @return Type of the iterated key.
    virtual NativeType type() const = 0;

    /// Get the contained value for a key as long.
    /// @return Retrieved value contained for the iterated key.
    virtual long getLong() const = 0;

    /// Get the contained value for a key as double.
    /// @return Retrieved value contained for the iterated key.
    virtual double getDouble() const = 0;

    /// Get the contained value for a key as float.
    /// @return Retrieved value contained for the iterated key.
    virtual float getFloat() const = 0;

    /// Get the contained value for a key as string.
    ///
    /// This should be possible for all key types.
    /// @return Retrieved value contained for the iterated key.
    virtual std::string getString() const = 0;

    /// Get the contained values for a key as array of long.
    /// @return Retrieved values contained for the iterated key.
    virtual std::vector<long> getLongArray() const = 0;

    /// Get the contained values for a key as array of double.
    /// @return Retrieved values contained for the iterated key.
    virtual std::vector<double> getDoubleArray() const = 0;

    /// Get the contained values for a key as array of float.
    /// @return Retrieved values contained for the iterated key.
    virtual std::vector<float> getFloatArray() const = 0;

    /// Get the contained values for a key as array of string.
    /// @return Retrieved values contained for the iterated key.
    virtual std::vector<std::string> getStringArray() const = 0;

    /// Get the contained values for a key as array of uint8_t (bytes).
    /// @return Retrieved values contained for the iterated key.
    virtual std::vector<std::uint8_t> getBytes() const = 0;


protected:

    /// Iterates the next element.
    virtual void next() = 0;

    /// Check if the iterator is valid.
    /// @return True if the iterator is still valid and holds a value. False indicates end of iteration.
    virtual bool isValid() const = 0;
};


//----------------------------------------------------------------------------------------------------------------------

/// Implements the key iterator (codes_key_iterator) based on a C++ range.
/// Enables use of range-based for-loop.
class KeyRange {
public:

    struct EndIterator;

    struct Iterator {
        friend struct EndIterator;

        Iterator& operator++() {
            state->next();
            return *this;
        };
        const KeyIterator& operator*() const { return *state; };
        const KeyIterator* operator->() const { return state; };

        inline bool operator!=(const KeyRange::EndIterator&) { return state->isValid(); }

        KeyIterator* state;
    };

    struct EndIterator {
        inline bool operator!=(const KeyRange::Iterator& it) { return it.state->isValid(); }
    };

    Iterator begin() const { return Iterator{impl_.get()}; };
    EndIterator end() const { return {}; };

    KeyRange(std::unique_ptr<KeyIterator> impl) : impl_{std::move(impl)} {};

private:

    std::unique_ptr<KeyIterator> impl_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
