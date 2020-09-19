/*
 * (C) Copyright 2017- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Aug 2017


#ifndef metkit_HyperCube_H
#define metkit_HyperCube_H

#include <iosfwd>
#include <map>
#include <memory>
#include <vector>

#include "eckit/utils/HyperCube.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsRequest.h"


namespace metkit {
namespace hypercube {

class Axis;

class HyperCube {
public:
    HyperCube(const metkit::mars::MarsRequest&);
    ~HyperCube();

    bool contains(const metkit::mars::MarsRequest&) const;
    bool clear(const metkit::mars::MarsRequest&);

    size_t count() const;

    size_t fieldOrdinal(const metkit::mars::MarsRequest&, bool noholes = true) const;
    metkit::mars::MarsRequest request() const;

protected:
    int indexOf(const metkit::mars::MarsRequest&) const;
    metkit::mars::MarsRequest requestOf(size_t index) const;

private:
    std::vector<Axis*> axes_;
    std::map<std::string, Axis*> axesByName_;
    std::vector<bool> set_;
    eckit::HyperCube cube_;
    size_t count_;


    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& s, const HyperCube& p) {
        p.print(s);
        return s;
    }
};

template <typename T>
class Deduplicator {
public:
    virtual bool empty(T) const = 0;
    virtual bool replace(T existing, T replacement) const = 0;
};

template <typename T>
class HyperCubeEntry {
public:
    HyperCubeEntry(const metkit::mars::MarsRequest& request,
                  T payload) :
        request_(request), payload_(payload) {}

    const metkit::mars::MarsRequest& request() const { return request_; }
    void payload(T p) { payload_ = p; }
    T payload() const { return payload_; }

private:
    metkit::mars::MarsRequest request_;
    T payload_;
};

template <typename T>
class HyperCubeContent : public HyperCube {
public:
    HyperCubeContent(const metkit::mars::MarsRequest& request, const Deduplicator<T>& deduplicator) :
        HyperCube(request), deduplicator_(deduplicator) {

        entries_ = std::vector<HyperCubeEntry<T>>();

        for (int i=0; i<count(); i++)
            entries_.push_back(HyperCubeEntry<T>(requestOf(i), T()));
    }


    void add(const metkit::mars::MarsRequest& request, T payload) {

        ASSERT(!deduplicator_.empty(payload));

        int idx = indexOf(request);

        ASSERT(0 <= idx);
        ASSERT(idx < entries_.size());

        if (deduplicator_.empty(entries_[idx].payload())) {
            entries_[idx].payload(payload);
        } else {
            if (deduplicator_.replace(entries_[idx].payload(), payload)) {
                entries_[idx].payload(payload);
            }
        }
    }

    const HyperCubeEntry<T>& at(size_t idx) {
        ASSERT(0 <= idx);
        ASSERT(idx < entries_.size());
        return entries_[idx];
    }

private:
    const Deduplicator<T>& deduplicator_;
    std::vector<HyperCubeEntry<T>> entries_;
};

}  // namespace hypercube
}  // namespace metkit


#endif
