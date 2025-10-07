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
#include <optional>
#include <string>
#include <variant>
#include <vector>


namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

class GeoRange;

/// Aggregate that stores a value with lon/lat.
struct GeoData {
    double value;
    double longitude;
    double latitude;
};

/// Abstract interface wrapping C API calls on on key_iterator.
class GeoIterator {
    friend class GeoRange;

public:

    virtual ~GeoIterator() = default;

    /// Access to the iterated data.
    /// @return Reference to the iterated geo data object.
    virtual const GeoData& data() const = 0;

    /// Check if there are follow up values.
    /// @return True if there are more values to iterate on.
    virtual bool hasNext() const = 0;


protected:

    /// Iterates the next element.
    virtual void next() = 0;

    /// Check if the iterator is valid.
    /// @return True if the iterator is still valid and holds a value. False indicates end of iteration.
    virtual bool isValid() const = 0;
};


//----------------------------------------------------------------------------------------------------------------------

/// Implements the geo iterator (codes_iterator) based on a C++ range.
/// Enables use of range-based for-loop.
class GeoRange {
public:

    struct EndIterator;

    struct Iterator {
        friend struct EndIterator;
        Iterator& operator++() {
            state->next();
            return *this;
        }
        const GeoData& operator*() const { return state->data(); }
        const GeoData* operator->() const { return &state->data(); }

        inline bool operator!=(const EndIterator&) { return state->isValid(); }

        GeoIterator* state;
    };

    struct EndIterator {
        inline bool operator!=(const Iterator& it) { return it.state->isValid(); }
    };


    Iterator begin() const { return Iterator{impl_.get()}; };
    EndIterator end() const { return {}; }

    GeoRange(std::unique_ptr<GeoIterator> impl) : impl_{std::move(impl)} {};

private:

    std::unique_ptr<GeoIterator> impl_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
