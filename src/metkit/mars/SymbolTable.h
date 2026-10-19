// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

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

template <typename S>
// requires std::is_arithmetic_v<S>
class SymbolTable {
public:

    /// @param kind what the dictionary holds ("keyword", "verb"): only used in error messages
    explicit SymbolTable(std::string kind = "keyword") : kind_(std::move(kind)) {
        static const std::string empty{};
        names_.push_back(empty);  // index 0 is reserved: it means "not found"
    }
    explicit SymbolTable(MemFile& file, std::string kind = "keyword");

    S add(const std::string& name);
    void addAlias(const std::string& name, S symbol);

    S symbol(const std::string& name, bool throwOnUnknown = true) const;
    const std::string& name(S symbol) const;

    size_t size() const;

    void write(std::ofstream& file) const;

private:

    void print(std::ostream&) const;

    std::string kind_;
    mutable std::shared_mutex mutex_;
    std::vector<std::reference_wrapper<const std::string>> names_;
    std::unordered_map<std::string, S> map_;

    friend std::ostream& operator<<(std::ostream& s, const SymbolTable<S>& dict) {
        dict.print(s);
        return s;
    }
};

template <typename S>
SymbolTable<S>::SymbolTable(MemFile& file, std::string kind) : kind_(std::move(kind)) {
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
        addAlias(name, static_cast<S>(file.read32()));
    }
}

template <typename S>
S SymbolTable<S>::add(const std::string& name) {
    // static_assert(std::is_integral_v<typename S::value_t> == true);

    // fast path: most calls are lookups of names that are already registered, avoid the exclusive lock
    if (S symb = symbol(name, false); symb != S{}) {
        return symb;
    }

    std::unique_lock lock(mutex_);

    auto it = map_.find(name);
    if (it != map_.end()) {  // someone else registered it between the two locks
        return it->second;
    }
    // the largest index must stay representable as S (and size() must not wrap around to 0)
    if (names_.size() >= static_cast<size_t>(std::numeric_limits<S>::max())) {
        std::ostringstream ss;
        ss << "Cannot register " << kind_ << " '" << name << "': dictionary is full (max "
           << static_cast<size_t>(std::numeric_limits<S>::max()) - 1 << " entries)";
        throw eckit::SeriousBug(ss.str(), Here());
    }
    bool inserted;
    S symbol{static_cast<typename S::value_t>(names_.size())};
    std::tie(it, inserted) = map_.emplace(name, symbol);
    ASSERT(inserted);
    names_.push_back(it->first);
    return symbol;
}

template <typename S>
void SymbolTable<S>::addAlias(const std::string& name, S symbol) {
    std::unique_lock lock(mutex_);
    if (symbol == S{} || symbol >= names_.size()) {
        std::ostringstream ss;
        ss << "Alias '" << name << "' refers to an invalid " << kind_ << " index " << static_cast<size_t>(symbol);
        throw eckit::SeriousBug(ss.str(), Here());
    }
    auto it = map_.find(name);
    if (it != map_.end()) {
        if (it->second != symbol) {
            std::ostringstream ss;
            ss << "Alias " << name << " conflicts with existing symbol " << names_[it->second].get();
            throw eckit::SeriousBug(ss.str(), Here());
        }
    }
    else {
        map_.emplace(name, symbol);
    }
}


template <typename S>
S SymbolTable<S>::symbol(const std::string& name, bool throwOnUnknown) const {
    std::shared_lock lock(mutex_);
    auto it = map_.find(name);
    if (it != map_.end()) {
        return it->second;
    }
    if (throwOnUnknown) {
        // an unknown name comes from the user (a typo, a keyword that does not exist): not an internal error
        throw eckit::UserError("Unknown " + kind_ + " '" + name + "'", Here());
    }
    return S{};
}

template <typename S>
const std::string& SymbolTable<S>::name(S symbol) const {
    // static_assert(std::is_integral_v<typename S::value_t> == true);

    std::shared_lock lock(mutex_);
    if (symbol == S{} || symbol >= names_.size()) {
        std::ostringstream ss;
        ss << "Invalid " << kind_ << " index " << static_cast<size_t>(symbol);
        throw eckit::SeriousBug(ss.str(), Here());
    }
    return names_[symbol].get();
}

template <typename S>
void SymbolTable<S>::print(std::ostream& s) const {
    std::shared_lock lock(mutex_);
    s << "[";
    for (size_t i = 0; i < names_.size(); ++i) {
        if (i > 0)
            s << ", ";
        s << names_[i].get() << ": " << i;
    }
    s << "]";
}

template <typename S>
size_t SymbolTable<S>::size() const {
    std::shared_lock lock(mutex_);
    return names_.size();
}

template <typename S>
void SymbolTable<S>::write(std::ofstream& file) const {
    std::shared_lock lock(mutex_);
    std::map<std::string, S> map(map_.begin(), map_.end());

    write32(file, names_.size() - 1);
    for (size_t i = 1; i < names_.size(); ++i) {
        writeString(file, names_[i]);
        map.erase(names_[i]);
    }
    write32(file, map.size());
    for (const auto& [name, symbol] : map) {
        writeString(file, name);
        write32(file, static_cast<uint32_t>(symbol));
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
