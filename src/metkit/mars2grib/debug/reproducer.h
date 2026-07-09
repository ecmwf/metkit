#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <iomanip>
#include <type_traits>
#include <stdexcept>

namespace metkit::mars2grib::debug::reproducer {

constexpr size_t BINARY_THRESHOLD = 5000;
constexpr uint32_t ENDIAN_SENTINEL = 0x12345678;

// -----------------------------------------------------------------------------
// Base Operation Interface
// -----------------------------------------------------------------------------
class Operation {
public:
    virtual ~Operation() = default;
    virtual void emitC_Statics(std::ostream& out, int id) const {}
    virtual void emitC_Logic(std::ostream& out, int id) const = 0;
};

// -----------------------------------------------------------------------------
// Lifecycle Operations (The "Commit" Flush)
// -----------------------------------------------------------------------------
class OpFromSample : public Operation {
    std::string sample_;
public:
    explicit OpFromSample(std::string s) : sample_(std::move(s)) {}
    void emitC_Logic(std::ostream& out, int /*id*/) const override {
        out << "    h = codes_grib_handle_new_from_samples(0, \"" << sample_ << "\");\n"
            << "    if(!h) { fprintf(stderr, \"Failed to create from sample\\n\"); exit(1); }\n";
    }
};

class OpClone : public Operation {
public:
    void emitC_Logic(std::ostream& out, int /*id*/) const override {
        out << "    // --- FLUSH/COMMIT BARRIER (Clone) ---\n"
            << "    {\n"
            << "        codes_handle* temp_h = codes_handle_clone(h);\n"
            << "        if(!temp_h) { fprintf(stderr, \"Clone flush failed\\n\"); exit(1); }\n"
            << "        codes_handle_delete(h);\n"
            << "        h = temp_h;\n"
            << "    }\n";
    }
};

// -----------------------------------------------------------------------------
// Missing Value
// -----------------------------------------------------------------------------
class OpMissing : public Operation {
    std::string key_;
public:
    explicit OpMissing(std::string k) : key_(std::move(k)) {}
    void emitC_Logic(std::ostream& out, int /*id*/) const override {
        out << "    CODES_CHECK(codes_set_missing(h, \"" << key_ << "\"), 0);\n";
    }
};

// -----------------------------------------------------------------------------
// Scalars 
// -----------------------------------------------------------------------------
template <typename T>
class OpScalar : public Operation {
    std::string key_;
    T val_;
public:
    OpScalar(std::string k, T v) : key_(std::move(k)), val_(std::move(v)) {}
    void emitC_Logic(std::ostream& out, int /*id*/) const override {
        out << "    CODES_CHECK(codes_set_";
        if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, long>) { 
            out << "long(h, \"" << key_ << "\", " << static_cast<long>(val_) << "), 0);\n"; 
        }
        else if constexpr (std::is_same_v<T, double>) { 
            out << "double(h, \"" << key_ << "\", " << std::setprecision(17) << val_ << "), 0);\n"; 
        }
        else if constexpr (std::is_same_v<T, std::string>) { 
            out << "string(h, \"" << key_ << "\", \"" << val_ << "\", NULL), 0);\n"; 
        }
    }
};

// -----------------------------------------------------------------------------
// Small Arrays (Inlined C statics)
// -----------------------------------------------------------------------------
template <typename T>
class OpSmallArray : public Operation {
    std::string key_;
    std::vector<T> vals_;
public:
    OpSmallArray(std::string k, const T* data, size_t size) 
        : key_(std::move(k)), vals_(data, data + size) {}
    
    void emitC_Statics(std::ostream& out, int id) const override {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, uint8_t>) return;

        out << "static ";
        if constexpr (std::is_same_v<T, long>) out << "long ";
        else out << "double ";
        
        out << "arr_" << id << "[] = {\n    ";
        for (size_t i = 0; i < vals_.size(); ++i) {
            if constexpr (std::is_same_v<T, double>) out << std::setprecision(17);
            out << vals_[i] << (i + 1 == vals_.size() ? "" : ", ");
            if ((i + 1) % 10 == 0 && i + 1 != vals_.size()) out << "\n    ";
        }
        out << "\n};\n";
    }

    void emitC_Logic(std::ostream& out, int id) const override {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, uint8_t>) {
            out << "    // WARNING: Array emission for key '" << key_ << "' not fully implemented.\n";
            return;
        }
        out << "    CODES_CHECK(codes_set_";
        if constexpr (std::is_same_v<T, long>) out << "long_array";
        else out << "double_array";
        out << "(h, \"" << key_ << "\", arr_" << id << ", " << vals_.size() << "), 0);\n";
    }
};

// -----------------------------------------------------------------------------
// Large Arrays (Zero-copy binary write)
// -----------------------------------------------------------------------------
template <typename T>
class OpLargeArray : public Operation {
    std::string key_;
    std::string bin_filename_;
public:
    OpLargeArray(std::string k, const T* data, size_t size, int id) 
        : key_(std::move(k)), bin_filename_("data_" + std::to_string(id) + ".bin") 
    {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, uint8_t>) return;

        std::ofstream bin(bin_filename_, std::ios::binary);
        if (!bin) throw std::runtime_error("Failed to open " + bin_filename_);

        uint32_t sentinel = ENDIAN_SENTINEL;
        uint32_t type_sz = sizeof(T);
        uint64_t n_elem = size;

        bin.write(reinterpret_cast<const char*>(&sentinel), 4);
        bin.write(reinterpret_cast<const char*>(&type_sz), 4);
        bin.write(reinterpret_cast<const char*>(&n_elem), 8);
        bin.write(reinterpret_cast<const char*>(data), n_elem * type_sz);
    }

    void emitC_Logic(std::ostream& out, int /*id*/) const override {
        if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, uint8_t>) return;

        out << "    {\n"
            << "        size_t n;\n"
            << "        void* ptr = load_payload(\"" << bin_filename_ << "\", &n, " << sizeof(T) << ");\n"
            << "        CODES_CHECK(codes_set_";
        if constexpr (std::is_same_v<T, long>) out << "long_array";
        else out << "double_array";
        out << "(h, \"" << key_ << "\", (";
        if constexpr (std::is_same_v<T, long>) out << "long*)ptr";
        else out << "double*)ptr";
        out << ", n), 0);\n"
            << "        free(ptr);\n"
            << "    }\n";
    }
};

// -----------------------------------------------------------------------------
// The Shared Recorder Core
// -----------------------------------------------------------------------------
class Recorder {
    std::vector<std::unique_ptr<Operation>> ops_;
    int op_counter_ = 0;

public:
    Recorder() = default;

    void record_from_sample(const std::string& sample_name) {
        ops_.push_back(std::make_unique<OpFromSample>(sample_name));
    }

    void record_clone() {
        ops_.push_back(std::make_unique<OpClone>());
    }

    void record_missing(const std::string& key) {
        ops_.push_back(std::make_unique<OpMissing>(key));
    }

    template <typename T>
    void record(const std::string& key, T val) {
        ops_.push_back(std::make_unique<OpScalar<T>>(key, std::move(val)));
    }

    template <typename T>
    void record_array(const std::string& key, const T* data, size_t size) {
        if (size > BINARY_THRESHOLD) {
            ops_.push_back(std::make_unique<OpLargeArray<T>>(key, data, size, ++op_counter_));
        } else {
            ops_.push_back(std::make_unique<OpSmallArray<T>>(key, data, size));
            ++op_counter_;
        }
    }

    void generate(const std::string& filename) const {
        std::ofstream out(filename);
        if (!out) throw std::runtime_error("Cannot open reproducer file.");

        out << R"C_CODE(#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <eccodes.h>

void* load_payload(const char* fn, size_t* n, size_t expected_sz) {
    FILE* f = fopen(fn, "rb");
    if(!f) { perror(fn); exit(1); }
    uint32_t s, sz; uint64_t c;
    if (fread(&s, 4, 1, f) != 1 || fread(&sz, 4, 1, f) != 1 || fread(&c, 8, 1, f) != 1) exit(1);
    if(sz != expected_sz) exit(1);
    
    void* d = malloc(c * sz);
    if (fread(d, sz, c, f) != c) exit(1);
    fclose(f);

    if(s == 0x78563412) {
        char* ptr = (char*)d;
        for (size_t i = 0; i < c * sz; i += sz) {
            for (size_t j = 0; j < sz / 2; ++j) {
                char tmp = ptr[i + j];
                ptr[i + j] = ptr[i + sz - 1 - j];
                ptr[i + sz - 1 - j] = tmp;
            }
        }
    } else if (s != 0x12345678) exit(1);
    *n = (size_t)c;
    return d;
}
)C_CODE";

        for (size_t i = 0; i < ops_.size(); ++i) ops_[i]->emitC_Statics(out, i);

        out << "\nint main() {\n"
            << "    codes_handle* h = NULL;\n\n";

        for (size_t i = 0; i < ops_.size(); ++i) ops_[i]->emitC_Logic(out, i);

        out << "\n    if(h) codes_handle_delete(h);\n"
            << "    return 0;\n}\n";
    }
};

} // namespace metkit::mars2grib::debug::reproducer