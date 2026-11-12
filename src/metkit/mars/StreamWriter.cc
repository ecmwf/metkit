#include "metkit/mars/StreamWriter.h"

#include "eckit/codec/detail/Endian.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/ByteSwap.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>
#include <cstring>
#include <limits>

namespace {

void swapEndian(uint16_t* v) {
    if (eckit::codec::Endian::native == eckit::codec::Endian::big) {
        *v = eckit::bitswap16(*v);
    }
}
void swapEndian(uint32_t* v) {
    if (eckit::codec::Endian::native == eckit::codec::Endian::big) {
        *v = eckit::bitswap32(*v);
    }
}

}  // namespace


void writeBool(std::ofstream& file, bool val) {
    uint8_t v = val ? 1 : 0;
    file.write(reinterpret_cast<const char*>(&v), sizeof(uint8_t));
}
void write8(std::ofstream& file, uint8_t size) {
    file.write(reinterpret_cast<char*>(&size), sizeof(uint8_t));
}
void write16(std::ofstream& file, uint16_t size) {
    swapEndian(&size);
    file.write(reinterpret_cast<char*>(&size), sizeof(uint16_t));
}
void write32(std::ofstream& file, uint32_t size) {
    swapEndian(&size);
    file.write(reinterpret_cast<char*>(&size), sizeof(uint32_t));
}
void write8(std::ofstream& file, size_t size) {
    if (size > static_cast<size_t>(std::numeric_limits<uint8_t>::max())) {
        std::ostringstream oss;
        oss << "TypeParam: cannot write params.bin - count of " << size << " exceeds the maximum of "
            << static_cast<size_t>(std::numeric_limits<uint8_t>::max()) << " supported by the uint8_t field width";
        throw eckit::SeriousBug(oss.str(), Here());
    }
    uint8_t size8 = static_cast<uint8_t>(size);
    file.write(reinterpret_cast<const char*>(&size8), sizeof(uint8_t));
}
void write16(std::ofstream& file, size_t size) {
    if (size > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
        std::ostringstream oss;
        oss << "TypeParam: cannot write params.bin - count of " << size << " exceeds the maximum of "
            << static_cast<size_t>(std::numeric_limits<uint16_t>::max()) << " supported by the uint16_t field width";
        throw eckit::SeriousBug(oss.str(), Here());
    }
    uint16_t size16 = static_cast<uint16_t>(size);
    swapEndian(&size16);
    file.write(reinterpret_cast<char*>(&size16), sizeof(uint16_t));
}
void write32(std::ofstream& file, size_t size) {
    ASSERT(size <= static_cast<size_t>(std::numeric_limits<uint32_t>::max()));
    uint32_t size32 = static_cast<uint32_t>(size);
    swapEndian(&size32);
    file.write(reinterpret_cast<char*>(&size32), sizeof(uint32_t));
}
void writeString(std::ofstream& file, const std::string& str) {
    if (str.size() > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
        std::ostringstream oss;
        oss << "TypeParam: cannot write params.bin - string '" << str << "' has length " << str.size()
            << " which exceeds the maximum of " << static_cast<size_t>(std::numeric_limits<uint16_t>::max())
            << " supported by the uint16_t field width";
        throw eckit::SeriousBug(oss.str(), Here());
    }
    uint16_t size = static_cast<uint16_t>(str.size());
    swapEndian(&size);
    file.write(reinterpret_cast<char*>(&size), sizeof(uint16_t));
    file.write(str.data(), size);
}
void writeStringSet(std::ofstream& file, const std::set<std::string>& s) {
    if (s.size() > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
        std::ostringstream ss;
        ss << "writeStringSet: set size " << s.size() << " exceeds the maximum supported size of "
           << static_cast<size_t>(std::numeric_limits<uint16_t>::max());
        throw eckit::SeriousBug(ss.str(), Here());
    }
    write16(file, s.size());
    for (const auto& str : s) {
        writeString(file, str);
    }
}
void writeStringVector(std::ofstream& file, const std::vector<std::string>& v) {
    if (v.size() > static_cast<size_t>(std::numeric_limits<uint16_t>::max())) {
        std::ostringstream ss;
        ss << "writeStringVector: vector size " << v.size() << " exceeds the maximum supported size of "
           << static_cast<size_t>(std::numeric_limits<uint16_t>::max());
        throw eckit::SeriousBug(ss.str(), Here());
    }
    write16(file, v.size());
    for (const auto& str : v) {
        writeString(file, str);
    }
}
