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

class GeoIterator;

struct GeoData {
    double value;
    double longitude;
    double latitude;
};

/// Abstract interface wrapping C API calls on on key_iterator
class IteratedGeoData {
    friend class GeoIterator;

public:  // methods

    /// Returns the name of the key
    virtual GeoData data() const = 0;

    virtual bool hasNext() const = 0;

    virtual ~IteratedGeoData() {};

protected:

    /// Iterates the next element.
    /// \return `true` if a next element is given. If `false` is returned, the iterator is invalidated
    virtual void next() = 0;

    /// Check if the iterator is valid
    virtual bool isValid() const = 0;
};


//----------------------------------------------------------------------------------------------------------------------

/// Implements the key iterator based on a C++ range
class GeoIterator {
public:

    struct Sentinel {};

    struct Iterator {
        Iterator& operator++() {
            state->next();
            return *this;
        }
        const GeoData operator*() const { return state->data(); }
        const GeoData operator->() const { return state->data(); }

        inline bool operator!=(const Sentinel&) { return state->isValid(); }

        IteratedGeoData* state;
    };


    Iterator begin() const { return Iterator{impl_.get()}; };
    Sentinel end() const { return {}; }

    GeoIterator(std::unique_ptr<IteratedGeoData> impl) : impl_{std::move(impl)} {};

private:

    std::unique_ptr<IteratedGeoData> impl_;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
