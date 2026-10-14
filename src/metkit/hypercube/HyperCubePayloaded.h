// SPDX-FileCopyrightText: 2017- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

/// @author Emanuele Danovaro
/// @date   Aug 2020


#pragma once

#include "metkit/hypercube/HyperCube.h"


namespace metkit::hypercube {

template <typename T>
class Deduplicator {
public:

    virtual bool toReplace(const T& existing, const T& replacement) const = 0;
};

template <typename T>
class HyperCubePayloaded : public HyperCube {
public:

    HyperCubePayloaded(const metkit::mars::MarsRequest& request, const Deduplicator<T>& deduplicator) :
        HyperCube(request), dedup_(deduplicator) {


        // throw error for default dedup
    }


    void add(const metkit::mars::MarsRequest& request, T payload) {

        int idx = indexOf(request);

        ASSERT(0 <= idx && idx < size());

        auto entry = entries_.find(idx);
        if (entry == entries_.end()) {
            entries_.emplace(idx, payload);
            clear(idx);
        }
        else {
            if (dedup_.toReplace(entry->second, payload)) {
                entry->second = payload;
            }
        }
    }

    const T& at(size_t idx) {
        ASSERT(0 <= idx);
        ASSERT(idx < size());

        return entries_[idx];
    }

    bool find(size_t idx, T& payload) {
        auto it = entries_.find(idx);

        if (it == entries_.end())
            return false;

        payload = it->second;
        return true;
    }

private:

    const Deduplicator<T>& dedup_;
    std::map<size_t, T> entries_;
};

}  // namespace metkit::hypercube
