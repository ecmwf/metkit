/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// File Serialize.h
// Emanuele Danovaro - ECMWF September 2026

#pragma once

#include <fstream>
#include <set>
#include <vector>

class MemFile {
public:

    MemFile() = default;
    MemFile(std::string filename);
    ~MemFile();

    MemFile& operator=(MemFile&&);

    uint8_t read8();
    uint16_t read16();
    uint32_t read32();
    std::string_view readString();
    std::string_view readString(size_t length);
    std::set<std::string> readStringSet();
    std::vector<std::string> readStringVector();

    void seek(off_t pos);

private:

    uint8_t* data_;
    size_t size_;
    off_t pos_;
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
