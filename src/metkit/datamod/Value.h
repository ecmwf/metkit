/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

/// @file Value.h
/// @brief A typed value of a datamod::Dict.
///
/// Experimental replacement for eckit::Value in the mars2grib hot paths: plain value semantics, no
/// reference counting and no shared global state. The alternatives mirror the types the encoder reads
/// from eckit::LocalConfiguration (scalars, homogeneous lists, nested dictionaries); `null` is a value
/// of its own, as in JSON.

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>
#include <vector>

namespace metkit::datamod {

class Dict;

struct Null {
    friend bool operator==(Null, Null) noexcept { return true; }
    friend bool operator!=(Null, Null) noexcept { return false; }
};

class Value {
public:

    using IntList    = std::vector<std::int64_t>;
    using DoubleList = std::vector<double>;
    using StringList = std::vector<std::string>;
    using DictList   = std::vector<Dict>;

    /// The alternatives, in the order of the underlying variant.
    enum class Type {
        Null,
        Bool,
        Int,
        Double,
        String,
        IntList,
        DoubleList,
        StringList,
        Dict,
        DictList,
    };

    Value() noexcept;
    Value(Null) noexcept;
    Value(bool value) noexcept;
    Value(int value) noexcept;
    Value(long value) noexcept;
    Value(long long value) noexcept;
    Value(float value) noexcept;
    Value(double value) noexcept;
    Value(const char* value);
    Value(std::string value) noexcept;
    Value(std::string_view value);
    Value(IntList value) noexcept;
    /// Integer vectors of other element types (e.g. int, or long where int64_t is long long).
    template <typename Int,
              std::enable_if_t<
                  std::is_integral_v<Int> && !std::is_same_v<Int, bool> && !std::is_same_v<Int, std::int64_t>, int> = 0>
    Value(const std::vector<Int>& value) : data_{IntList(value.begin(), value.end())} {}
    Value(DoubleList value) noexcept;
    Value(const std::vector<float>& value);
    Value(StringList value) noexcept;
    Value(Dict value);
    Value(DictList value);

    Value(const Value& other);
    Value(Value&& other) noexcept;
    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;
    ~Value();

    Type type() const noexcept { return static_cast<Type>(data_.index()); }
    static std::string_view typeName(Type type) noexcept;

    bool isNull() const noexcept { return type() == Type::Null; }
    bool isBool() const noexcept { return type() == Type::Bool; }
    bool isInt() const noexcept { return type() == Type::Int; }
    bool isDouble() const noexcept { return type() == Type::Double; }
    bool isString() const noexcept { return type() == Type::String; }
    bool isIntList() const noexcept { return type() == Type::IntList; }
    bool isDoubleList() const noexcept { return type() == Type::DoubleList; }
    bool isStringList() const noexcept { return type() == Type::StringList; }
    bool isDict() const noexcept { return type() == Type::Dict; }
    bool isDictList() const noexcept { return type() == Type::DictList; }

    /// Number of elements of a list (0 for other types).
    std::size_t listSize() const noexcept;

    // Strict accessors: throw eckit::BadValue if the value has another type.
    bool asBool() const;
    std::int64_t asInt() const;
    double asDouble() const;
    const std::string& asString() const;
    const IntList& asIntList() const;
    const DoubleList& asDoubleList() const;
    const StringList& asStringList() const;
    const Dict& asDict() const;
    Dict& asDict();
    const DictList& asDictList() const;

    friend bool operator==(const Value& a, const Value& b);
    friend bool operator!=(const Value& a, const Value& b) { return !(a == b); }

private:

    [[noreturn]] void wrongType(Type wanted) const;

    // Dictionaries are boxed so the variant stays small and Dict can be incomplete here.
    using Data = std::variant<Null, bool, std::int64_t, double, std::string, IntList, DoubleList, StringList,
                              std::unique_ptr<Dict>, std::unique_ptr<DictList>>;
    Data data_;
};

}  // namespace metkit::datamod
