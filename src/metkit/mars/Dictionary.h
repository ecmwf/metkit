/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Emanuele Danovaro

/// @date Sep 2026

#pragma once

#include <memory>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "eckit/exception/Exceptions.h"

namespace metkit::mars {

using Verb    = uint8_t;
using Keyword = uint16_t;

template <typename K>
// requires std::is_arithmetic_v<K>
class Dictionary {
public:

    Dictionary() {
        static const std::string empty{};
        names_.push_back(empty);
    }

    K add(const std::string& name);
    void alias(const std::string& name, K key);

    K keyword(const std::string& name) const;
    K exist(const std::string& name) const;

    const std::string& name(K key) const;

    K size() const;

private:

    void print(std::ostream&) const;

    mutable std::shared_mutex mutex_;
    std::vector<std::reference_wrapper<const std::string>> names_;
    std::unordered_map<std::string, K> map_;

    friend std::ostream& operator<<(std::ostream& s, const Dictionary<K>& dict) {
        dict.print(s);
        return s;
    }
};

template <typename K>
K Dictionary<K>::add(const std::string& name) {
    static_assert(std::is_integral_v<K> == true);

    std::unique_lock lock(mutex_);

    auto it = map_.find(name);
    if (it != map_.end()) {
        return it->second;
    }
    bool inserted;
    K key                  = names_.size();
    std::tie(it, inserted) = map_.emplace(name, key);
    ASSERT(inserted);
    names_.push_back(it->first);
    return key;
}
template <typename K>
void Dictionary<K>::alias(const std::string& name, K key) {
    std::unique_lock lock(mutex_);
    auto it = map_.find(name);
    if (it != map_.end()) {
        if (it->second != key) {
            std::ostringstream ss;
            ss << "Alias " << name << " conflicts with existing key " << names_[it->second].get();
            throw eckit::SeriousBug(ss.str(), Here());
        }
    }
    else {
        map_.emplace(name, key);
    }
}

template <typename K>
K Dictionary<K>::exist(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = map_.find(name);
    if (it != map_.end()) {
        return it->second;
    }
    return 0;
}
template <typename K>
K Dictionary<K>::keyword(const std::string& name) const {
    K key = exist(name);
    if (key) {
        return key;
    }
    std::shared_lock lock(mutex_);
    std::ostringstream ss;
    ss << "Unknown keyword '" << name << "'";
    ss << " (valid keywords are: ";
    for (const auto& k : names_) {
        ss << k.get() << " ";
    }
    ss << ")";
    throw eckit::SeriousBug(ss.str(), Here());
}

template <typename K>
const std::string& Dictionary<K>::name(K key) const {
    static_assert(std::is_integral_v<K> == true);

    std::shared_lock lock(mutex_);
    if (key == 0 || key >= names_.size()) {
        std::ostringstream ss;
        ss << "Invalid keyword index " << key;
        throw eckit::SeriousBug(ss.str(), Here());
    }
    return names_[key].get();
}

template <typename K>
void Dictionary<K>::print(std::ostream& s) const {
    std::shared_lock lock(mutex_);
    s << "[";
    for (size_t i = 0; i < names_.size(); ++i) {
        if (i > 0)
            s << ", ";
        s << names_[i].get() << ": " << i;
    }
    s << "]";
}

template <typename K>
K Dictionary<K>::size() const {
    std::shared_lock lock(mutex_);
    return static_cast<K>(names_.size());
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
