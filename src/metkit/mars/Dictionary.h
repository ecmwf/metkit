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

#include <cstdint>
#include <fstream>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <shared_mutex>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "eckit/exception/Exceptions.h"

#include "Serialize.h"

namespace metkit::mars {

using Verb    = uint8_t;
using Keyword = uint16_t;

template <typename K>
// requires std::is_arithmetic_v<K>
class Dictionary {
public:

    /// @param kind what the dictionary holds ("keyword", "verb"): only used in error messages
    explicit Dictionary(std::string kind = "keyword") : kind_(std::move(kind)) {
        static const std::string empty{};
        names_.push_back(empty);  // index 0 is reserved: it means "not found"
    }
    explicit Dictionary(MemFile& file, std::string kind = "keyword");

    K add(const std::string& name);
    void alias(const std::string& name, K key);

    K keyword(const std::string& name) const;
    K exist(const std::string& name) const;

    const std::string& name(K key) const;

    K size() const;

    void serialize(std::ofstream& file) const;

private:

    void print(std::ostream&) const;

    std::string kind_;
    mutable std::shared_mutex mutex_;
    std::vector<std::reference_wrapper<const std::string>> names_;
    std::unordered_map<std::string, K> map_;

    friend std::ostream& operator<<(std::ostream& s, const Dictionary<K>& dict) {
        dict.print(s);
        return s;
    }
};

template <typename K>
Dictionary<K>::Dictionary(MemFile& file, std::string kind) : kind_(std::move(kind)) {
    static const std::string empty{};
    names_.push_back(empty);
    size_t num = file.read32();
    for (size_t i = 0; i < num; ++i) {
        add(std::string{file.readString()});
    }
    // read aliases
    num = file.read32();
    for (size_t i = 0; i < num; ++i) {
        std::string name{file.readString()};
        alias(name, static_cast<K>(file.read32()));
    }
}

template <typename K>
K Dictionary<K>::add(const std::string& name) {
    static_assert(std::is_integral_v<K> == true);

    // fast path: most calls are lookups of names that are already registered, avoid the exclusive lock
    if (K key = exist(name)) {
        return key;
    }

    std::unique_lock lock(mutex_);

    auto it = map_.find(name);
    if (it != map_.end()) {  // someone else registered it between the two locks
        return it->second;
    }
    // the largest index must stay representable as K (and size() must not wrap around to 0)
    if (names_.size() >= static_cast<size_t>(std::numeric_limits<K>::max())) {
        std::ostringstream ss;
        ss << "Cannot register " << kind_ << " '" << name << "': dictionary is full (max "
           << static_cast<size_t>(std::numeric_limits<K>::max()) - 1 << " entries)";
        throw eckit::SeriousBug(ss.str(), Here());
    }
    bool inserted;
    K key                  = static_cast<K>(names_.size());
    std::tie(it, inserted) = map_.emplace(name, key);
    ASSERT(inserted);
    names_.push_back(it->first);
    return key;
}

template <typename K>
void Dictionary<K>::alias(const std::string& name, K key) {
    std::unique_lock lock(mutex_);
    if (key == 0 || key >= names_.size()) {
        std::ostringstream ss;
        ss << "Alias '" << name << "' refers to an invalid " << kind_ << " index " << static_cast<size_t>(key);
        throw eckit::SeriousBug(ss.str(), Here());
    }
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
    // an unknown name comes from the user (a typo, a keyword that does not exist): not an internal error
    throw eckit::UserError("Unknown " + kind_ + " '" + name + "'", Here());
}

template <typename K>
const std::string& Dictionary<K>::name(K key) const {
    static_assert(std::is_integral_v<K> == true);

    std::shared_lock lock(mutex_);
    if (key == 0 || key >= names_.size()) {
        std::ostringstream ss;
        ss << "Invalid " << kind_ << " index " << static_cast<size_t>(key);
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

template <typename K>
void Dictionary<K>::serialize(std::ofstream& file) const {
    std::shared_lock lock(mutex_);  // taken before copying map_, which is shared with concurrent add() calls
    std::map<std::string, K> map(map_.begin(), map_.end());

    write32(file, names_.size() - 1);
    for (size_t i = 1; i < names_.size(); ++i) {
        writeString(file, names_[i]);
        map.erase(names_[i]);
    }
    write32(file, map.size());
    for (const auto& [name, key] : map) {
        writeString(file, name);
        write32(file, static_cast<uint32_t>(key));
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
