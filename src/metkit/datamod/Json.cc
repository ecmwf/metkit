/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/datamod/Json.h"

#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <utility>
#include <vector>

#include "eckit/exception/Exceptions.h"

namespace metkit::datamod {

namespace {

//----------------------------------------------------------------------------------------------------------------------
// Reader

class Reader {
public:

    explicit Reader(std::string_view text) : text_{text} {}

    Value document() {
        Value value = parseValue();
        skipSpaces();
        if (pos_ != text_.size()) {
            fail("unexpected characters after the JSON value");
        }
        return value;
    }

private:

    [[noreturn]] void fail(const std::string& what) const {
        throw eckit::BadValue("Invalid JSON at offset " + std::to_string(pos_) + ": " + what, Here());
    }

    void skipSpaces() {
        while (pos_ < text_.size() &&
               (text_[pos_] == ' ' || text_[pos_] == '\t' || text_[pos_] == '\n' || text_[pos_] == '\r')) {
            ++pos_;
        }
    }

    char peek() {
        skipSpaces();
        if (pos_ >= text_.size()) {
            fail("unexpected end of input");
        }
        return text_[pos_];
    }

    void expect(char c) {
        if (peek() != c) {
            fail(std::string{"expected '"} + c + "'");
        }
        ++pos_;
    }

    void literal(std::string_view word) {
        if (text_.substr(pos_, word.size()) != word) {
            fail("invalid literal");
        }
        pos_ += word.size();
    }

    Value parseValue() {
        switch (peek()) {
            case '{':
                return Value{parseObject()};
            case '[':
                return parseArray();
            case '"':
                return Value{parseString()};
            case 't':
                literal("true");
                return Value{true};
            case 'f':
                literal("false");
                return Value{false};
            case 'n':
                literal("null");
                return Value{Null{}};
            default:
                return parseNumber();
        }
    }

    Dict parseObject() {
        expect('{');
        Dict dict;
        if (peek() == '}') {
            ++pos_;
            return dict;
        }
        while (true) {
            if (peek() != '"') {
                fail("expected a key");
            }
            std::string key = parseString();
            expect(':');
            dict.set(key, parseValue());
            const char c = peek();
            ++pos_;
            if (c == '}') {
                return dict;
            }
            if (c != ',') {
                fail("expected ',' or '}'");
            }
        }
    }

    Value parseArray() {
        expect('[');
        std::vector<Value> items;
        if (peek() == ']') {
            ++pos_;
            return Value{Value::IntList{}};
        }
        while (true) {
            items.push_back(parseValue());
            const char c = peek();
            ++pos_;
            if (c == ']') {
                break;
            }
            if (c != ',') {
                fail("expected ',' or ']'");
            }
        }
        return homogeneous(std::move(items));
    }

    Value homogeneous(std::vector<Value> items) {
        bool ints = true, numbers = true, strings = true, dicts = true;
        for (const auto& item : items) {
            ints    = ints && item.isInt();
            numbers = numbers && (item.isInt() || item.isDouble());
            strings = strings && item.isString();
            dicts   = dicts && item.isDict();
        }
        if (ints) {
            Value::IntList out;
            out.reserve(items.size());
            for (const auto& item : items) {
                out.push_back(item.asInt());
            }
            return Value{std::move(out)};
        }
        if (numbers) {
            Value::DoubleList out;
            out.reserve(items.size());
            for (const auto& item : items) {
                out.push_back(item.isInt() ? static_cast<double>(item.asInt()) : item.asDouble());
            }
            return Value{std::move(out)};
        }
        if (strings) {
            Value::StringList out;
            out.reserve(items.size());
            for (auto& item : items) {
                out.push_back(item.asString());
            }
            return Value{std::move(out)};
        }
        if (dicts) {
            Value::DictList out;
            out.reserve(items.size());
            for (auto& item : items) {
                out.push_back(std::move(item.asDict()));
            }
            return Value{std::move(out)};
        }
        fail("unsupported array (only homogeneous arrays of numbers, strings or objects)");
    }

    static void appendUtf8(std::string& out, unsigned long cp) {
        if (cp < 0x80) {
            out += static_cast<char>(cp);
        }
        else if (cp < 0x800) {
            out += static_cast<char>(0xC0 | (cp >> 6));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
        else if (cp < 0x10000) {
            out += static_cast<char>(0xE0 | (cp >> 12));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
        else {
            out += static_cast<char>(0xF0 | (cp >> 18));
            out += static_cast<char>(0x80 | ((cp >> 12) & 0x3F));
            out += static_cast<char>(0x80 | ((cp >> 6) & 0x3F));
            out += static_cast<char>(0x80 | (cp & 0x3F));
        }
    }

    unsigned long hex4() {
        if (pos_ + 4 > text_.size()) {
            fail("truncated \\u escape");
        }
        unsigned long value = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = text_[pos_++];
            value <<= 4;
            if (c >= '0' && c <= '9') {
                value |= static_cast<unsigned long>(c - '0');
            }
            else if (c >= 'a' && c <= 'f') {
                value |= static_cast<unsigned long>(c - 'a' + 10);
            }
            else if (c >= 'A' && c <= 'F') {
                value |= static_cast<unsigned long>(c - 'A' + 10);
            }
            else {
                fail("invalid \\u escape");
            }
        }
        return value;
    }

    std::string parseString() {
        expect('"');
        std::string out;
        while (true) {
            if (pos_ >= text_.size()) {
                fail("unterminated string");
            }
            // Copy the run of plain characters at once.
            const std::size_t start = pos_;
            while (pos_ < text_.size() && text_[pos_] != '"' && text_[pos_] != '\\') {
                ++pos_;
            }
            out.append(text_.data() + start, pos_ - start);
            if (pos_ >= text_.size()) {
                fail("unterminated string");
            }
            if (text_[pos_] == '"') {
                ++pos_;
                return out;
            }
            ++pos_;  // backslash
            if (pos_ >= text_.size()) {
                fail("unterminated escape");
            }
            const char c = text_[pos_++];
            switch (c) {
                case '"':
                case '\\':
                case '/':
                    out += c;
                    break;
                case 'b':
                    out += '\b';
                    break;
                case 'f':
                    out += '\f';
                    break;
                case 'n':
                    out += '\n';
                    break;
                case 'r':
                    out += '\r';
                    break;
                case 't':
                    out += '\t';
                    break;
                case 'u': {
                    unsigned long cp = hex4();
                    if (cp >= 0xD800 && cp <= 0xDBFF && text_.substr(pos_, 2) == "\\u") {
                        pos_ += 2;
                        const unsigned long low = hex4();
                        cp                      = 0x10000 + ((cp - 0xD800) << 10) + (low - 0xDC00);
                    }
                    appendUtf8(out, cp);
                    break;
                }
                default:
                    fail("invalid escape");
            }
        }
    }

    Value parseNumber() {
        const std::size_t start = pos_;
        bool integral           = true;
        if (pos_ < text_.size() && text_[pos_] == '-') {
            ++pos_;
        }
        while (pos_ < text_.size()) {
            const char c = text_[pos_];
            if (c >= '0' && c <= '9') {
                ++pos_;
            }
            else if (c == '.' || c == 'e' || c == 'E' || c == '+' || c == '-') {
                integral = false;
                ++pos_;
            }
            else {
                break;
            }
        }
        if (pos_ == start || (pos_ == start + 1 && text_[start] == '-')) {
            fail("invalid value");
        }
        const std::string number{text_.substr(start, pos_ - start)};
        char* end = nullptr;
        errno     = 0;
        if (integral) {
            const long long value = std::strtoll(number.c_str(), &end, 10);
            if (errno == 0 && *end == '\0') {
                return Value{value};
            }
            errno = 0;  // out of range: read as a double
        }
        const double value = std::strtod(number.c_str(), &end);
        if (*end != '\0') {
            fail("invalid number `" + number + "`");
        }
        return Value{value};
    }

    std::string_view text_;
    std::size_t pos_{0};
};

//----------------------------------------------------------------------------------------------------------------------
// Writer

void writeString(std::string& out, const std::string& value) {
    out += '"';
    for (const char c : value) {
        switch (c) {
            case '"':
                out += "\\\"";
                break;
            case '\\':
                out += "\\\\";
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
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned>(c));
                    out += buffer;
                }
                else {
                    out += c;
                }
        }
    }
    out += '"';
}

void writeDouble(std::string& out, double value) {
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%.17g", value);
    out += buffer;
    // Keep it a double when read back.
    if (std::isfinite(value) && std::strpbrk(buffer, ".eE") == nullptr) {
        out += ".0";
    }
}

void writeValue(std::string& out, const Value& value);

void writeDict(std::string& out, const Dict& dict) {
    out += '{';
    bool first = true;
    for (const auto& entry : dict.entries()) {
        if (!first) {
            out += ',';
        }
        first = false;
        writeString(out, entry.key);
        out += ':';
        writeValue(out, entry.value);
    }
    out += '}';
}

template <typename List, typename Write>
void writeList(std::string& out, const List& list, Write&& write) {
    out += '[';
    for (std::size_t i = 0; i < list.size(); ++i) {
        if (i != 0) {
            out += ',';
        }
        write(list[i]);
    }
    out += ']';
}

void writeValue(std::string& out, const Value& value) {
    switch (value.type()) {
        case Value::Type::Null:
            out += "null";
            break;
        case Value::Type::Bool:
            out += value.asBool() ? "true" : "false";
            break;
        case Value::Type::Int:
            out += std::to_string(value.asInt());
            break;
        case Value::Type::Double:
            writeDouble(out, value.asDouble());
            break;
        case Value::Type::String:
            writeString(out, value.asString());
            break;
        case Value::Type::IntList:
            writeList(out, value.asIntList(), [&](std::int64_t v) { out += std::to_string(v); });
            break;
        case Value::Type::DoubleList:
            writeList(out, value.asDoubleList(), [&](double v) { writeDouble(out, v); });
            break;
        case Value::Type::StringList:
            writeList(out, value.asStringList(), [&](const std::string& v) { writeString(out, v); });
            break;
        case Value::Type::Dict:
            writeDict(out, value.asDict());
            break;
        case Value::Type::DictList:
            writeList(out, value.asDictList(), [&](const Dict& v) { writeDict(out, v); });
            break;
    }
}

}  // namespace

Value parseJson(std::string_view json) {
    return Reader{json}.document();
}

Dict parseJsonObject(std::string_view json) {
    Value value = parseJson(json);
    if (!value.isDict()) {
        throw eckit::BadValue("JSON document is not an object", Here());
    }
    return std::move(value.asDict());
}

std::string toJson(const Dict& dict) {
    std::string out;
    writeDict(out, dict);
    return out;
}

std::string toJson(const Value& value) {
    std::string out;
    writeValue(out, value);
    return out;
}

}  // namespace metkit::datamod
