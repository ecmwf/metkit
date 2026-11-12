// SPDX-FileCopyrightText: 2026- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

// File StreamWriter.h
// Emanuele Danovaro - ECMWF September 2026

#pragma once

#include <sys/types.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <set>
#include <string>
#include <vector>

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
