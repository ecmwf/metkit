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
class HyperCubeEntry {
public:
    HyperCubeEntry(metkit::mars::MarsRequest request,
                  std::chrono::system_clock::time_point timestamp,
                  T payload) :
        request_(request), timestamp_(timestamp), payload_(payload) {}

    const metkit::mars::MarsRequest& request() const { return request_; }
    const std::chrono::system_clock::time_point& timestamp() const { return timestamp_; }
    T payload() const { return payload_; }

//    bool operator < (const HyperCubeData& el) const { return true; }

private:
    metkit::mars::MarsRequest request_;
    std::chrono::system_clock::time_point timestamp_;
    T payload_;
};


template <typename T>
class HyperCubePlusPayload : public HyperCube {
public:
    HyperCubePlusPayload(const metkit::mars::MarsRequest& request) : HyperCube(request) {

        entries_ = std::vector<HyperCubeEntry<T>>();

        for (int i=0; i<count(); i++)
            entries_.push_back(HyperCubeEntry<T>(requestOf(i), std::chrono::system_clock::time_point{}, nullptr));
    }


    void add(const HyperCubeEntry<T>& entry) {

        int idx = indexOf(entry.request());

        ASSERT(0 <= idx);
        ASSERT(idx < entries_.size());

        if (entries_[idx].payload() == nullptr) {
            entries_[idx] = entry;
        } else {
            if (entries_[idx].timestamp() < entry.timestamp()) {
                printOlder(entries_[idx], entry);
                entries_[idx] = entry;
            } else {
                printOlder(entry, entries_[idx]);
            }
        }
    }

    const HyperCubeEntry<T>& at(size_t idx) {
        ASSERT(0 <= idx);
        ASSERT(idx < entries_.size());
        return entries_[idx];
    }

private:
    static void printOlder(const HyperCubeEntry<T>& older, const HyperCubeEntry<T>& newer) {
        std::cout << "Dropping payload associated with request " << older.request() << " since its timestamp is " << std::chrono::system_clock::to_time_t(older.timestamp())
                  << " < " << std::chrono::system_clock::to_time_t(newer.timestamp()) << std::endl;
    }

private:
    std::vector<HyperCubeEntry<T>> entries_;
};

}  // namespace hypercube
}  // namespace metkit


#endif
