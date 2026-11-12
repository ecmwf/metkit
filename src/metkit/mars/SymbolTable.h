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

#include "metkit/mars/MMapFileReader.h"
#include "metkit/mars/StreamWriter.h"

namespace metkit::mars {

/// Thread-safe bidirectional dictionary between names and compact integer-like symbols (e.g. MarsKeyword, MarsVerb).
/// Symbol 0 is reserved for "unknown";
/// Several names (aliases) can map to the same symbol
/// A symbol resolves back to its primary name.
/// Can be serialised to / loaded from a MMapFileReader.
template <typename S>
// requires std::is_arithmetic_v<S>
class SymbolTable {
public:

    /// @param kind what the dictionary holds ("keyword", "verb"): only used in error messages
    explicit SymbolTable() {
        static const std::string empty{};
        names_.push_back(empty);  // index 0 is reserved: it means "not found"
    }
    explicit SymbolTable(MMapFileReader& file);

    S add(const std::string& name);
    void addAlias(const std::string& name, S symbol);

    S symbol(const std::string& name) const;
    S symbol_or_default(const std::string& name) const noexcept;
    const std::string& name(S symbol) const;

    size_t size() const;

    void write(std::ofstream& file) const;

private:

    void print(std::ostream&) const;

    mutable std::shared_mutex mutex_;
    std::vector<std::reference_wrapper<const std::string>> names_;
    std::unordered_map<std::string, S> map_;

    friend std::ostream& operator<<(std::ostream& s, const SymbolTable<S>& dict) {
        dict.print(s);
        return s;
    }
};

template <typename S>
SymbolTable<S>::SymbolTable(MMapFileReader& file) {
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

    // fast path: most calls are lookups of names that are already registered, avoid the exclusive lock
    if (S symb = symbol_or_default(name); symb != S{}) {
        return symb;
    }

    std::unique_lock lock(mutex_);

    S symbol{static_cast<typename S::value_t>(names_.size())};
    auto [it, inserted] = map_.emplace(name, symbol);

    if (!inserted) {  // someone else registered it between the retrieval and the lock
        return it->second;
    }

    // the largest index must stay representable as S (and size() must not wrap around to 0)
    if (names_.size() >= static_cast<size_t>(std::numeric_limits<S>::max())) {
        std::ostringstream ss;
        ss << "Cannot register " << (typeid(S{}).name()) << " '" << name << "': dictionary is full (max "
           << static_cast<size_t>(std::numeric_limits<S>::max()) - 1 << " entries)";
        map_.erase(it);
        throw eckit::SeriousBug(ss.str(), Here());
    }
    names_.push_back(it->first);
    return symbol;
}

template <typename S>
void SymbolTable<S>::addAlias(const std::string& name, S symbol) {
    std::unique_lock lock(mutex_);
    if (symbol == S{} || symbol >= names_.size()) {
        std::ostringstream ss;
        ss << "Alias '" << name << "' refers to an invalid " << (typeid(S{}).name()) << " index "
           << static_cast<size_t>(symbol);
        throw eckit::SeriousBug(ss.str(), Here());
    }
    auto [it, inserted] = map_.emplace(name, symbol);
    if (!inserted) {
        if (it->second != symbol) {
            std::ostringstream ss;
            ss << "Alias " << name << " conflicts with existing symbol " << names_[it->second].get();
            throw eckit::SeriousBug(ss.str(), Here());
        }
    }
}


template <typename S>
S SymbolTable<S>::symbol(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = map_.find(name);
    if (it != map_.end()) {
        return it->second;
    }

    // the user requested an unknown name
    std::ostringstream ss;
    ss << "Unknown " << (typeid(S{}).name()) << " '" << name << "'";
    throw eckit::UserError(ss.str(), Here());
}

template <typename S>
S SymbolTable<S>::symbol_or_default(const std::string& name) const noexcept {
    std::shared_lock lock(mutex_);
    auto it = map_.find(name);
    if (it != map_.end()) {
        return it->second;
    }

    return S{};
}

template <typename S>
const std::string& SymbolTable<S>::name(S symbol) const {
    std::shared_lock lock(mutex_);
    if (symbol != S{} && !(symbol >= names_.size())) {
        return names_[symbol].get();
    }

    std::ostringstream ss;
    ss << "Invalid " << (typeid(S{}).name()) << " index " << static_cast<size_t>(symbol);
    throw eckit::SeriousBug(ss.str(), Here());
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
