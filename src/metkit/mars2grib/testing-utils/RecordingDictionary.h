#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

namespace metkit::mars2grib::testing_utils {

namespace detail {

inline std::string escape_json(std::string_view input) {
    std::string out;
    out.reserve(input.size() + 8);

    for (char c : input) {
        switch (c) {
            case '\\':
                out += "\\\\";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\b':
                out += "\\b";
                break;
            case '\f':
                out += "\\f";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    std::ostringstream oss;
                    oss << "\\u";
                    oss.width(4);
                    oss.fill('0');
                    oss << std::hex << static_cast<int>(static_cast<unsigned char>(c));
                    out += oss.str();
                }
                else {
                    out += c;
                }
                break;
        }
    }

    return out;
}

inline std::string quote_json(std::string_view input) {
    std::string out;
    out.reserve(input.size() + 2);
    out += '"';
    out += escape_json(input);
    out += '"';
    return out;
}

template <typename T>
inline std::string numeric_to_json(const T& value) {
    std::ostringstream oss;
    oss.precision(17);
    oss << value;
    return oss.str();
}

template <typename T>
inline std::string vector_to_json(const std::vector<T>& values);

template <typename T>
struct RecordedTypeTraits;

template <>
struct RecordedTypeTraits<bool> {
    static constexpr std::string_view category = "bool";

    static std::string datatype_json(bool) {
        return R"({"type":"boolean","rank":0,"size":1})";
    }

    static std::string value_json(bool value) {
        return value ? "true" : "false";
    }
};

template <>
struct RecordedTypeTraits<long long> {
    static constexpr std::string_view category = "integer";

    static std::string datatype_json(long long) {
        return R"({"type":"integer","rank":0,"size":1})";
    }

    static std::string value_json(long long value) {
        return numeric_to_json(value);
    }
};

template <>
struct RecordedTypeTraits<double> {
    static constexpr std::string_view category = "double";

    static std::string datatype_json(double) {
        return R"({"type":"double","rank":0,"size":1})";
    }

    static std::string value_json(double value) {
        return numeric_to_json(value);
    }
};

template <>
struct RecordedTypeTraits<std::string> {
    static constexpr std::string_view category = "string";

    static std::string datatype_json(const std::string&) {
        return R"({"type":"string","rank":0,"size":1})";
    }

    static std::string value_json(const std::string& value) {
        return quote_json(value);
    }
};

template <>
struct RecordedTypeTraits<std::vector<long long>> {
    static constexpr std::string_view category = "vector<integer>";

    static std::string datatype_json(const std::vector<long long>& value) {
        return std::string{"{\"type\":\"integer\",\"rank\":1,\"size\":"} + numeric_to_json(value.size()) +
               '}';
    }

    static std::string value_json(const std::vector<long long>& value) {
        return vector_to_json(value);
    }
};

template <>
struct RecordedTypeTraits<std::vector<double>> {
    static constexpr std::string_view category = "vector<double>";

    static std::string datatype_json(const std::vector<double>& value) {
        return std::string{"{\"type\":\"double\",\"rank\":1,\"size\":"} + numeric_to_json(value.size()) +
               '}';
    }

    static std::string value_json(const std::vector<double>& value) {
        return vector_to_json(value);
    }
};

template <>
struct RecordedTypeTraits<std::vector<std::string>> {
    static constexpr std::string_view category = "vector<string>";

    static std::string datatype_json(const std::vector<std::string>& value) {
        return std::string{"{\"type\":\"string\",\"rank\":1,\"size\":"} + numeric_to_json(value.size()) +
               '}';
    }

    static std::string value_json(const std::vector<std::string>& value) {
        return vector_to_json(value);
    }
};

template <>
struct RecordedTypeTraits<std::vector<uint8_t>> {
    static constexpr std::string_view category = "vector<byte>";

    static std::string datatype_json(const std::vector<uint8_t>& value) {
        return std::string{"{\"type\":\"byte\",\"rank\":1,\"size\":"} + numeric_to_json(value.size()) + '}';
    }

    static std::string value_json(const std::vector<uint8_t>& value) {
        std::vector<long long> normalized;
        normalized.reserve(value.size());
        for (uint8_t item : value) {
            normalized.push_back(static_cast<long long>(item));
        }
        return vector_to_json(normalized);
    }
};

template <typename T>
inline std::string element_to_json(const T& value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return quote_json(value);
    }
    else {
        return numeric_to_json(value);
    }
}

template <>
inline std::string element_to_json<bool>(const bool& value) {
    return value ? "true" : "false";
}

template <typename T>
inline std::string vector_to_json(const std::vector<T>& values) {
    std::string out;
    out += '[';
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i != 0) {
            out += ',';
        }
        out += element_to_json(values[i]);
    }
    out += ']';
    return out;
}

}  // namespace detail


class RecordingDictionary {
public:
    struct ValueCompareResult {
        bool equal{true};
        std::string path;
        std::string reason;
        std::string lhs_json;
        std::string rhs_json;
    };

    struct CompareResult {
        bool equal{true};
        std::size_t first_mismatch_index{std::numeric_limits<std::size_t>::max()};
        std::string path;
        std::string reason;
        std::string lhs_json;
        std::string rhs_json;

        explicit operator bool() const {
            return equal;
        }
    };

    class RecordedValue {
    public:
        virtual ~RecordedValue() = default;

        virtual std::unique_ptr<RecordedValue> clone() const = 0;
        virtual std::string category() const                 = 0;
        virtual std::string datatype_json() const            = 0;
        virtual std::string value_json() const               = 0;
        virtual ValueCompareResult compare(const RecordedValue& other) const = 0;
    };

    template <typename T>
    class TypedRecordedValue final : public RecordedValue {
    public:
        explicit TypedRecordedValue(T value) : value_{std::move(value)} {}

        std::unique_ptr<RecordedValue> clone() const override {
            return std::make_unique<TypedRecordedValue<T>>(value_);
        }

        std::string category() const override {
            return std::string{detail::RecordedTypeTraits<T>::category};
        }

        std::string datatype_json() const override {
            return detail::RecordedTypeTraits<T>::datatype_json(value_);
        }

        std::string value_json() const override {
            return detail::RecordedTypeTraits<T>::value_json(value_);
        }

        ValueCompareResult compare(const RecordedValue& other) const override {
            const auto* typedOther = dynamic_cast<const TypedRecordedValue<T>*>(&other);
            if (typedOther == nullptr) {
                return {false,
                        "datatype",
                        "Recorded value categories differ",
                        datatype_json(),
                        other.datatype_json()};
            }

            if (!(value_ == typedOther->value_)) {
                return {false,
                        "value",
                        "Recorded values differ",
                        value_json(),
                        typedOther->value_json()};
            }

            return {};
        }

        const T& value() const {
            return value_;
        }

    private:
        T value_;
    };

    struct SampleMetadata {
        std::string sample;

        bool operator==(const SampleMetadata& other) const {
            return sample == other.sample;
        }
    };

    struct CloneMetadata {
        std::size_t source_operation_count{0};

        bool operator==(const CloneMetadata& other) const {
            return source_operation_count == other.source_operation_count;
        }
    };

    struct MissingValueTag {
        bool operator==(const MissingValueTag&) const {
            return true;
        }
    };

    struct ValuesSummary {
        std::size_t size{0};
        double average{0.0};

        bool operator==(const ValuesSummary& other) const {
            return size == other.size && average == other.average;
        }
    };

public:
    enum class OperationKind {
        MakeFromSample,
        Clone,
        Set,
        SetMissing,
    };

    struct Operation {
        OperationKind kind{OperationKind::Set};
        std::string key;
        std::unique_ptr<const RecordedValue> value;

        Operation() = default;

        Operation(OperationKind opKind, std::string opKey, std::unique_ptr<const RecordedValue> opValue) :
            kind{opKind},
            key{std::move(opKey)},
            value{std::move(opValue)} {}

        Operation(const Operation& other) : kind{other.kind}, key{other.key} {
            if (other.value) {
                value = other.value->clone();
            }
        }

        Operation& operator=(const Operation& other) {
            if (this == &other) {
                return *this;
            }

            kind = other.kind;
            key  = other.key;
            value.reset();
            if (other.value) {
                value = other.value->clone();
            }
            return *this;
        }

        Operation(Operation&&) noexcept            = default;
        Operation& operator=(Operation&&) noexcept = default;
    };

    RecordingDictionary() = default;
    RecordingDictionary(const RecordingDictionary&)            = default;
    RecordingDictionary& operator=(const RecordingDictionary&) = default;
    RecordingDictionary(RecordingDictionary&&) noexcept        = default;
    RecordingDictionary& operator=(RecordingDictionary&&) noexcept = default;
    ~RecordingDictionary()                                     = default;

    void record_make_from_sample(std::string_view sample) {
        operations_.emplace_back(OperationKind::MakeFromSample, std::string{},
                                 std::make_unique<TypedRecordedValue<SampleMetadata>>(SampleMetadata{std::string(sample)}));
    }

    void record_clone(std::size_t sourceOperationCount) {
        operations_.emplace_back(
            OperationKind::Clone, std::string{},
            std::make_unique<TypedRecordedValue<CloneMetadata>>(CloneMetadata{sourceOperationCount}));
    }

    void record_set_missing(std::string_view key) {
        operations_.emplace_back(OperationKind::SetMissing, std::string(key),
                                 std::make_unique<TypedRecordedValue<MissingValueTag>>(MissingValueTag{}));
    }

    void record_set(std::string_view key, bool value) {
        operations_.emplace_back(OperationKind::Set, std::string(key), std::make_unique<TypedRecordedValue<bool>>(value));
    }

    void record_set(std::string_view key, int value) {
        record_set(key, static_cast<long long>(value));
    }

    void record_set(std::string_view key, long value) {
        record_set(key, static_cast<long long>(value));
    }

    void record_set(std::string_view key, long long value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<long long>>(value));
    }

    void record_set(std::string_view key, double value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<double>>(value));
    }

    void record_set(std::string_view key, const std::string& value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::string>>(value));
    }

    void record_set(std::string_view key, std::string&& value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::string>>(std::move(value)));
    }

    void record_set(std::string_view key, const char* value) {
        record_set(key, std::string(value));
    }

    void record_set(std::string_view key, const std::vector<long>& value) {
        std::vector<long long> normalized;
        normalized.reserve(value.size());
        for (long item : value) {
            normalized.push_back(static_cast<long long>(item));
        }

        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::vector<long long>>>(std::move(normalized)));
    }

    void record_set(std::string_view key, std::vector<long>&& value) {
        record_set(key, static_cast<const std::vector<long>&>(value));
    }

    void record_set(std::string_view key, const std::vector<double>& value) {
        if (key == "values") {
            operations_.emplace_back(OperationKind::Set, std::string(key),
                                     std::make_unique<TypedRecordedValue<ValuesSummary>>(summarize_values(value)));
            return;
        }

        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::vector<double>>>(value));
    }

    void record_set(std::string_view key, std::vector<double>&& value) {
        if (key == "values") {
            operations_.emplace_back(OperationKind::Set, std::string(key),
                                     std::make_unique<TypedRecordedValue<ValuesSummary>>(summarize_values(value)));
            return;
        }

        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::vector<double>>>(std::move(value)));
    }

    void record_set(std::string_view key, const std::vector<std::string>& value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::vector<std::string>>>(value));
    }

    void record_set(std::string_view key, std::vector<std::string>&& value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::vector<std::string>>>(std::move(value)));
    }

    void record_set(std::string_view key, const std::vector<uint8_t>& value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::vector<uint8_t>>>(value));
    }

    void record_set(std::string_view key, std::vector<uint8_t>&& value) {
        operations_.emplace_back(OperationKind::Set, std::string(key),
                                 std::make_unique<TypedRecordedValue<std::vector<uint8_t>>>(std::move(value)));
    }

    std::size_t operation_count() const {
        return operations_.size();
    }

    const std::vector<Operation>& operations() const {
        return operations_;
    }

    std::string operation_to_json(const Operation& operation) const {
        switch (operation.kind) {
            case OperationKind::MakeFromSample: {
                return std::string{"{\"make_from_sample\":{\"sample\":"} + operation.value->value_json() + "}}";
            }
            case OperationKind::Clone: {
                return std::string{"{\"clone\":{\"source_operation_count\":"} + operation.value->value_json() + "}}";
            }
            case OperationKind::Set: {
                return std::string{"{\"set\":{\"key\":"} + detail::quote_json(operation.key) +
                       ",\"datatype\":" + operation.value->datatype_json() + ",\"value\":" +
                       operation.value->value_json() + "}}";
            }
            case OperationKind::SetMissing: {
                return std::string{"{\"set_missing\":{\"key\":"} + detail::quote_json(operation.key) + "}}";
            }
        }

        return "{}";
    }

    std::string to_json() const {
        std::string out;
        out += "{\"operations\":[";
        for (std::size_t i = 0; i < operations_.size(); ++i) {
            if (i != 0) {
                out += ',';
            }
            out += operation_to_json(operations_[i]);
        }
        out += "]}";
        return out;
    }

    CompareResult compare(const RecordingDictionary& other) const {
        if (operations_.size() != other.operations_.size()) {
            return {false,
                    std::min(operations_.size(), other.operations_.size()),
                    "operations.size",
                    "Recorded operation counts differ",
                    detail::numeric_to_json(operations_.size()),
                    detail::numeric_to_json(other.operations_.size())};
        }

        for (std::size_t i = 0; i < operations_.size(); ++i) {
            const auto& lhs = operations_[i];
            const auto& rhs = other.operations_[i];

            if (lhs.kind != rhs.kind) {
                return {false,
                        i,
                        "operations[" + detail::numeric_to_json(i) + "].kind",
                        "Recorded operation kinds differ",
                        operation_to_json(lhs),
                        operation_to_json(rhs)};
            }

            if (lhs.key != rhs.key) {
                return {false,
                        i,
                        "operations[" + detail::numeric_to_json(i) + "].key",
                        "Recorded operation keys differ",
                        operation_to_json(lhs),
                        operation_to_json(rhs)};
            }

            if (static_cast<bool>(lhs.value) != static_cast<bool>(rhs.value)) {
                return {false,
                        i,
                        "operations[" + detail::numeric_to_json(i) + "].value",
                        "Recorded operation value presence differs",
                        operation_to_json(lhs),
                        operation_to_json(rhs)};
            }

            if (lhs.value) {
                ValueCompareResult valueCmp = lhs.value->compare(*rhs.value);
                if (!valueCmp.equal) {
                    return {false,
                            i,
                            "operations[" + detail::numeric_to_json(i) + "]." + valueCmp.path,
                            valueCmp.reason,
                            operation_to_json(lhs),
                            operation_to_json(rhs)};
                }
            }
        }

        return {};
    }

    bool operator==(const RecordingDictionary& other) const {
        return compare(other).equal;
    }

    bool operator!=(const RecordingDictionary& other) const {
        return !(*this == other);
    }

private:
    static ValuesSummary summarize_values(const std::vector<double>& value) {
        double sum = 0.0;

        for (double item : value) {
            if (std::isnan(item)) {
                throw std::invalid_argument("RecordingDictionary does not accept NaN in `values`");
            }
            sum += item;
        }

        const double average = value.empty() ? 0.0 : (sum / static_cast<double>(value.size()));
        return ValuesSummary{value.size(), average};
    }

    std::vector<Operation> operations_;
};


template <>
struct detail::RecordedTypeTraits<RecordingDictionary::SampleMetadata> {
    static constexpr std::string_view category = "sample-metadata";

    static std::string datatype_json(const RecordingDictionary::SampleMetadata&) {
        return R"({"type":"sample","rank":0,"size":1})";
    }

    static std::string value_json(const RecordingDictionary::SampleMetadata& value) {
        return detail::quote_json(value.sample);
    }
};

template <>
struct detail::RecordedTypeTraits<RecordingDictionary::CloneMetadata> {
    static constexpr std::string_view category = "clone-metadata";

    static std::string datatype_json(const RecordingDictionary::CloneMetadata&) {
        return R"({"type":"clone","rank":0,"size":1})";
    }

    static std::string value_json(const RecordingDictionary::CloneMetadata& value) {
        return detail::numeric_to_json(value.source_operation_count);
    }
};

template <>
struct detail::RecordedTypeTraits<RecordingDictionary::MissingValueTag> {
    static constexpr std::string_view category = "missing";

    static std::string datatype_json(const RecordingDictionary::MissingValueTag&) {
        return R"({"type":"missing","rank":0,"size":0})";
    }

    static std::string value_json(const RecordingDictionary::MissingValueTag&) {
        return "null";
    }
};

template <>
struct detail::RecordedTypeTraits<RecordingDictionary::ValuesSummary> {
    static constexpr std::string_view category = "values-summary";

    static std::string datatype_json(const RecordingDictionary::ValuesSummary& value) {
        return std::string{"{\"type\":\"double\",\"rank\":1,\"size\":"} + detail::numeric_to_json(value.size) +
               '}';
    }

    static std::string value_json(const RecordingDictionary::ValuesSummary& value) {
        return std::string{"{\"average\":"} + detail::numeric_to_json(value.average) + '}';
    }
};

}  // namespace metkit::mars2grib::testing_utils
