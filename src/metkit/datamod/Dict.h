/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Dict.h
/// @brief A flat string-keyed dictionary of datamod::Value.
///
/// Open-addressing hash table stored in two vectors: the entries in insertion order (key, cached hash,
/// value) and a power-of-two slot table of entry indices with linear probing. A lookup hashes the key
/// once and compares hashes before strings; nothing is shared between dictionaries, so copies and
/// lookups need no locks. Keys are plain names (no dotted paths).

#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "metkit/datamod/Value.h"

namespace metkit::datamod {

/// 64-bit FNV-1a hash of a key.
std::uint64_t hashKey(std::string_view key) noexcept;

class Dict {
public:

    struct Entry {
        std::string key;
        std::uint64_t hash{0};
        Value value;
    };

    Dict() = default;

    bool has(std::string_view key) const noexcept { return find(key) != nullptr; }

    const Value* find(std::string_view key) const noexcept;
    Value* find(std::string_view key) noexcept;

    /// The value of `key`; throws eckit::UserError if absent.
    const Value& at(std::string_view key) const;

    /// Insert or replace; returns the stored value.
    Value& set(std::string_view key, Value value);

    /// Remove `key`; returns false if it was absent.
    bool erase(std::string_view key);

    void clear() noexcept;

    std::size_t size() const noexcept { return entries_.size(); }
    bool empty() const noexcept { return entries_.empty(); }

    /// Entries in insertion order.
    const std::vector<Entry>& entries() const noexcept { return entries_; }

    /// Same keys with equal values (order does not matter).
    friend bool operator==(const Dict& a, const Dict& b);
    friend bool operator!=(const Dict& a, const Dict& b) { return !(a == b); }

private:

    /// Slot holding `key`, or the empty slot where it would go. Requires a non-empty slot table.
    std::size_t probe(std::string_view key, std::uint64_t hash) const noexcept;
    void rebuild(std::size_t capacity);

    std::vector<Entry> entries_;
    std::vector<std::uint32_t> slots_;  // 0 = empty, otherwise entry index + 1
};

}  // namespace metkit::datamod
