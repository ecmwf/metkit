// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File MMapFileReader.h
// Emanuele Danovaro - ECMWF September 2026

#pragma once

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <set>
#include <string>
#include <string_view>
#include <vector>

/// Read-only memory mapped file, read sequentially (and convert from little endian).
class MMapFileReader {
public:

    MMapFileReader() = default;
    explicit MMapFileReader(const std::string& filename);

    MMapFileReader(const MMapFileReader&)            = delete;
    MMapFileReader& operator=(const MMapFileReader&) = delete;
    MMapFileReader(MMapFileReader&& other) noexcept;
    MMapFileReader& operator=(MMapFileReader&& other) noexcept;

    ~MMapFileReader();

    uint8_t read8();
    uint16_t read16();
    uint32_t read32();
    std::string_view readString();
    std::string_view readString(size_t length);
    std::set<std::string> readStringSet();
    std::vector<std::string> readStringVector();

    void seek(off_t pos);

    /// true if everything has been read
    bool atEnd() const { return pos_ == size_; }

private:

    void release() noexcept;

    const uint8_t* data_ = nullptr;
    size_t size_         = 0;
    size_t pos_          = 0;
};

void writeBool(std::ofstream& file, bool val);
void write8(std::ofstream& file, uint8_t size);
void write8(std::ofstream& file, size_t size);
void write16(std::ofstream& file, uint16_t size);
void write16(std::ofstream& file, size_t size);
void write32(std::ofstream& file, uint32_t size);
void write32(std::ofstream& file, size_t size);
void writeString(std::ofstream& file, const std::string& str);
void writeStringSet(std::ofstream& file, const std::set<std::string>& s);
void writeStringVector(std::ofstream& file, const std::vector<std::string>& v);
