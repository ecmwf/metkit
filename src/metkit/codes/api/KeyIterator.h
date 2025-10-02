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

/// Valid namespaces for key iteration
enum Namespace : std::size_t {
    Ls,          /// "ls"
    Parameter,   /// "parameter"
    Statistics,  /// "statistics"
    Time,        /// "time"
    Geography,   /// "geography"
    Vertical,    /// "vertical"
    Mars,        /// "mars"
};

enum KeyIteratorFlags : unsigned int {
    AllKeys = 0,  // Default - Will iterate all keys in a namespace

    // Additional flags to filter on keys - to be combined with AllKeys
    SkipReadOnly        = 1 << 0,
    SkipOptional        = 1 << 1,
    SkipEditionSpecific = 1 << 2,
    SkipCoded           = 1 << 3,
    SkipComputed        = 1 << 4,
    SkipDuplicates      = 1 << 5,
    SkipFunction        = 1 << 6,
};

// Overload bitwise operators
KeyIteratorFlags operator|(KeyIteratorFlags a, KeyIteratorFlags b);
KeyIteratorFlags operator&(KeyIteratorFlags a, KeyIteratorFlags b);
KeyIteratorFlags operator^(KeyIteratorFlags a, KeyIteratorFlags b);
KeyIteratorFlags operator~(KeyIteratorFlags a);

// Optional helper to check if a flag is set
bool hasFlag(KeyIteratorFlags value, KeyIteratorFlags flag);


//----------------------------------------------------------------------------------------------------------------------

class KeyIterator;

/// Abstract interface wrapping C API calls on on key_iterator
class IteratedKey {
    friend class KeyIterator;

public:  // methods

    /// Returns the name of the key
    virtual std::string name() const = 0;

    /// Get the value of the key
    virtual Value get() const = 0;

    /// Get the type of the key
    virtual NativeType getType() const = 0;

    /// Explicit getters
    virtual long getLong() const          = 0;
    virtual double getDouble() const      = 0;
    virtual float getFloat() const        = 0;
    virtual std::string getString() const = 0;

    virtual std::vector<long> getLongArray() const          = 0;
    virtual std::vector<double> getDoubleArray() const      = 0;
    virtual std::vector<float> getFloatArray() const        = 0;
    virtual std::vector<std::string> getStringArray() const = 0;

    virtual std::vector<unsigned char> getBytes() const = 0;

    virtual ~IteratedKey() {};

protected:

    /// Iterates the next element.
    virtual void next() = 0;

    /// Check if the iterator is valid
    virtual bool isValid() const = 0;
};


//----------------------------------------------------------------------------------------------------------------------

/// Implements the key iterator based on a C++ range
class KeyIterator {
public:

    struct Sentinel {};

    struct Iterator {
        Iterator& operator++() {
            state->next();
            return *this;
        };
        const IteratedKey& operator*() const { return *state; };
        const IteratedKey* operator->() const { return state; };

        inline bool operator!=(const KeyIterator::Sentinel&) { return state->isValid(); }

        IteratedKey* state;
    };

    Iterator begin() const { return Iterator{impl_.get()}; };
    Sentinel end() const { return {}; };

    KeyIterator(std::unique_ptr<IteratedKey> impl) : impl_{std::move(impl)} {};

private:

    std::unique_ptr<IteratedKey> impl_;
};


//------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
