/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/datamod/Dict.h"

#include <type_traits>
#include <utility>

#include "eckit/exception/Exceptions.h"

namespace metkit::datamod {

//----------------------------------------------------------------------------------------------------------------------
// Value

namespace {

template <typename To, typename From>
std::vector<To> convertVector(const std::vector<From>& values) {
    return std::vector<To>(values.begin(), values.end());
}

}  // namespace

Value::Value() noexcept : data_{Null{}} {}
Value::Value(Null) noexcept : data_{Null{}} {}
Value::Value(bool value) noexcept : data_{value} {}
Value::Value(int value) noexcept : data_{static_cast<std::int64_t>(value)} {}
Value::Value(long value) noexcept : data_{static_cast<std::int64_t>(value)} {}
Value::Value(long long value) noexcept : data_{static_cast<std::int64_t>(value)} {}
Value::Value(float value) noexcept : data_{static_cast<double>(value)} {}
Value::Value(double value) noexcept : data_{value} {}
Value::Value(const char* value) : data_{std::string{value}} {}
Value::Value(std::string value) noexcept : data_{std::move(value)} {}
Value::Value(std::string_view value) : data_{std::string{value}} {}
Value::Value(IntList value) noexcept : data_{std::move(value)} {}
Value::Value(DoubleList value) noexcept : data_{std::move(value)} {}
Value::Value(const std::vector<float>& value) : data_{convertVector<double>(value)} {}
Value::Value(StringList value) noexcept : data_{std::move(value)} {}
Value::Value(Dict value) : data_{std::make_unique<Dict>(std::move(value))} {}
Value::Value(DictList value) : data_{std::make_unique<DictList>(std::move(value))} {}

Value::Value(const Value& other) : data_{Null{}} {
    *this = other;
}

Value::Value(Value&& other) noexcept = default;

Value& Value::operator=(const Value& other) {
    if (this == &other) {
        return *this;
    }
    switch (other.type()) {
        case Type::Dict:
            data_ = std::make_unique<Dict>(*std::get<std::unique_ptr<Dict>>(other.data_));
            break;
        case Type::DictList:
            data_ = std::make_unique<DictList>(*std::get<std::unique_ptr<DictList>>(other.data_));
            break;
        default:
            std::visit(
                [this](const auto& value) {
                    using T = std::decay_t<decltype(value)>;
                    if constexpr (!std::is_same_v<T, std::unique_ptr<Dict>> &&
                                  !std::is_same_v<T, std::unique_ptr<DictList>>) {
                        data_ = value;
                    }
                },
                other.data_);
            break;
    }
    return *this;
}

Value& Value::operator=(Value&& other) noexcept = default;

Value::~Value() = default;

std::string_view Value::typeName(Type type) noexcept {
    switch (type) {
        case Type::Null:
            return "null";
        case Type::Bool:
            return "bool";
        case Type::Int:
            return "integer";
        case Type::Double:
            return "double";
        case Type::String:
            return "string";
        case Type::IntList:
            return "integer list";
        case Type::DoubleList:
            return "double list";
        case Type::StringList:
            return "string list";
        case Type::Dict:
            return "dictionary";
        case Type::DictList:
            return "dictionary list";
    }
    return "unknown";
}

std::size_t Value::listSize() const noexcept {
    switch (type()) {
        case Type::IntList:
            return std::get<IntList>(data_).size();
        case Type::DoubleList:
            return std::get<DoubleList>(data_).size();
        case Type::StringList:
            return std::get<StringList>(data_).size();
        case Type::DictList:
            return std::get<std::unique_ptr<DictList>>(data_)->size();
        default:
            return 0;
    }
}

void Value::wrongType(Type wanted) const {
    throw eckit::BadValue(
        "datamod::Value is " + std::string{typeName(type())} + ", not " + std::string{typeName(wanted)}, Here());
}

bool Value::asBool() const {
    if (!isBool()) {
        wrongType(Type::Bool);
    }
    return std::get<bool>(data_);
}

std::int64_t Value::asInt() const {
    if (!isInt()) {
        wrongType(Type::Int);
    }
    return std::get<std::int64_t>(data_);
}

double Value::asDouble() const {
    if (!isDouble()) {
        wrongType(Type::Double);
    }
    return std::get<double>(data_);
}

const std::string& Value::asString() const {
    if (!isString()) {
        wrongType(Type::String);
    }
    return std::get<std::string>(data_);
}

const Value::IntList& Value::asIntList() const {
    if (!isIntList()) {
        wrongType(Type::IntList);
    }
    return std::get<IntList>(data_);
}

const Value::DoubleList& Value::asDoubleList() const {
    if (!isDoubleList()) {
        wrongType(Type::DoubleList);
    }
    return std::get<DoubleList>(data_);
}

const Value::StringList& Value::asStringList() const {
    if (!isStringList()) {
        wrongType(Type::StringList);
    }
    return std::get<StringList>(data_);
}

const Dict& Value::asDict() const {
    if (!isDict()) {
        wrongType(Type::Dict);
    }
    return *std::get<std::unique_ptr<Dict>>(data_);
}

Dict& Value::asDict() {
    if (!isDict()) {
        wrongType(Type::Dict);
    }
    return *std::get<std::unique_ptr<Dict>>(data_);
}

const Value::DictList& Value::asDictList() const {
    if (!isDictList()) {
        wrongType(Type::DictList);
    }
    return *std::get<std::unique_ptr<DictList>>(data_);
}

bool operator==(const Value& a, const Value& b) {
    if (a.type() != b.type()) {
        return false;
    }
    switch (a.type()) {
        case Value::Type::Dict:
            return a.asDict() == b.asDict();
        case Value::Type::DictList:
            return a.asDictList() == b.asDictList();
        default:
            return a.data_ == b.data_;
    }
}

//----------------------------------------------------------------------------------------------------------------------
// Dict

std::uint64_t hashKey(std::string_view key) noexcept {
    std::uint64_t hash = 14695981039346656037ULL;
    for (const char c : key) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::size_t Dict::probe(std::string_view key, std::uint64_t hash) const noexcept {
    const std::size_t mask = slots_.size() - 1;
    std::size_t slot       = static_cast<std::size_t>(hash) & mask;
    while (true) {
        const std::uint32_t index = slots_[slot];
        if (index == 0) {
            return slot;
        }
        const Entry& entry = entries_[index - 1];
        if (entry.hash == hash && entry.key == key) {
            return slot;
        }
        slot = (slot + 1) & mask;
    }
}

const Value* Dict::find(std::string_view key) const noexcept {
    if (entries_.empty()) {
        return nullptr;
    }
    const std::uint32_t index = slots_[probe(key, hashKey(key))];
    return index == 0 ? nullptr : &entries_[index - 1].value;
}

Value* Dict::find(std::string_view key) noexcept {
    return const_cast<Value*>(static_cast<const Dict&>(*this).find(key));
}

const Value& Dict::at(std::string_view key) const {
    const Value* value = find(key);
    if (value == nullptr) {
        throw eckit::UserError("datamod::Dict has no key `" + std::string{key} + "`", Here());
    }
    return *value;
}

Value& Dict::set(std::string_view key, Value value) {
    const std::uint64_t hash = hashKey(key);
    if (slots_.empty()) {
        rebuild(16);
    }
    std::size_t slot = probe(key, hash);
    if (slots_[slot] != 0) {
        Value& stored = entries_[slots_[slot] - 1].value;
        stored        = std::move(value);
        return stored;
    }
    // Keep the load factor at or below 1/2.
    if ((entries_.size() + 1) * 2 > slots_.size()) {
        rebuild(slots_.size() * 2);
        slot = probe(key, hash);
    }
    entries_.push_back(Entry{std::string{key}, hash, std::move(value)});
    slots_[slot] = static_cast<std::uint32_t>(entries_.size());
    return entries_.back().value;
}

bool Dict::erase(std::string_view key) {
    if (entries_.empty()) {
        return false;
    }
    const std::size_t slot = probe(key, hashKey(key));
    if (slots_[slot] == 0) {
        return false;
    }
    entries_.erase(entries_.begin() + (slots_[slot] - 1));
    rebuild(slots_.size());  // entry indices changed
    return true;
}

void Dict::clear() noexcept {
    entries_.clear();
    slots_.clear();
}

void Dict::rebuild(std::size_t capacity) {
    slots_.assign(capacity, 0);
    const std::size_t mask = capacity - 1;
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        std::size_t slot = static_cast<std::size_t>(entries_[i].hash) & mask;
        while (slots_[slot] != 0) {
            slot = (slot + 1) & mask;
        }
        slots_[slot] = static_cast<std::uint32_t>(i + 1);
    }
}

bool operator==(const Dict& a, const Dict& b) {
    if (a.size() != b.size()) {
        return false;
    }
    for (const auto& entry : a.entries_) {
        const Value* other = b.find(entry.key);
        if (other == nullptr || !(*other == entry.value)) {
            return false;
        }
    }
    return true;
}

}  // namespace metkit::datamod
