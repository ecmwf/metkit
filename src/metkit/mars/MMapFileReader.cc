#include "metkit/mars/MMapFileReader.h"

#include "eckit/codec/detail/Endian.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/utils/ByteSwap.h"

#include <fcntl.h>
#include <sys/mman.h>
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

namespace metkit::mars {

MMapFileReader::MMapFileReader(const std::string& filename) {
    int fd = ::open(filename.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        throw eckit::SeriousBug("Failed to open file: " + filename, Here());
    }

    struct stat sb;
    if (::fstat(fd, &sb) == -1) {
        ::close(fd);
        throw eckit::SeriousBug("Failed to query size of file: " + filename, Here());
    }
    if (sb.st_size <= 0) {  // mmap does not accept an empty mapping
        ::close(fd);
        throw eckit::SeriousBug("File is empty: " + filename, Here());
    }

    void* addr = ::mmap(nullptr, static_cast<size_t>(sb.st_size), PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd);  // the mapping stays valid after closing the file descriptor
    if (addr == MAP_FAILED) {
        throw eckit::SeriousBug("Failed to map file: " + filename, Here());
    }

    data_ = static_cast<const uint8_t*>(addr);
    size_ = static_cast<size_t>(sb.st_size);
}

MMapFileReader::MMapFileReader(MMapFileReader&& other) noexcept :
    data_(other.data_), size_(other.size_), pos_(other.pos_) {
    other.data_ = nullptr;
    other.size_ = 0;
    other.pos_  = 0;
}

MMapFileReader& MMapFileReader::operator=(MMapFileReader&& other) noexcept {
    if (this != &other) {
        release();  // do not leak the mapping we currently hold

        data_ = other.data_;
        size_ = other.size_;
        pos_  = other.pos_;

        other.data_ = nullptr;
        other.size_ = 0;
        other.pos_  = 0;
    }
    return *this;
}

MMapFileReader::~MMapFileReader() {
    release();
}

void MMapFileReader::release() noexcept {
    if (data_) {
        ::munmap(const_cast<uint8_t*>(data_), size_);
    }
    data_ = nullptr;
    size_ = 0;
    pos_  = 0;
}

// The reads use memcpy: the data is not aligned (strings have an arbitrary length), and casting the pointer to a
// wider type would be undefined behaviour (and a SIGBUS on some architectures).
uint8_t MMapFileReader::read8() {
    if (pos_ < size_) {
        return data_[pos_++];
    }
    throw eckit::SeriousBug("Failed to read 8-bit value from file", Here());
}
uint16_t MMapFileReader::read16() {
    if (size_ - pos_ >= sizeof(uint16_t)) {
        uint16_t value;
        std::memcpy(&value, data_ + pos_, sizeof(value));
        pos_ += sizeof(value);
        swapEndian(&value);
        return value;
    }
    throw eckit::SeriousBug("Failed to read 16-bit value from file", Here());
}
uint32_t MMapFileReader::read32() {
    if (size_ - pos_ >= sizeof(uint32_t)) {
        uint32_t value;
        std::memcpy(&value, data_ + pos_, sizeof(value));
        pos_ += sizeof(value);
        swapEndian(&value);
        return value;
    }
    throw eckit::SeriousBug("Failed to read 32-bit value from file", Here());
}
std::string_view MMapFileReader::readString() {
    uint16_t length = read16();
    if (size_ - pos_ >= length) {
        std::string_view result(reinterpret_cast<const char*>(data_ + pos_), length);
        pos_ += length;
        return result;
    }
    throw eckit::SeriousBug("Failed to read string from file", Here());
}
std::string_view MMapFileReader::readString(size_t length) {
    if (size_ - pos_ >= length) {
        std::string_view result(reinterpret_cast<const char*>(data_ + pos_), length);
        pos_ += length;
        return result;
    }
    throw eckit::SeriousBug("Failed to read string of specified length from file", Here());
}

std::set<std::string> MMapFileReader::readStringSet() {
    uint16_t count = read16();
    std::set<std::string> result;
    for (uint16_t i = 0; i < count; ++i) {
        result.emplace(readString());
    }
    return result;
}

std::vector<std::string> MMapFileReader::readStringVector() {
    uint16_t count = read16();
    std::vector<std::string> result;
    result.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
        result.emplace_back(readString());
    }
    return result;
}

void MMapFileReader::seek(off_t pos) {
    if (pos >= 0 && static_cast<size_t>(pos) <= size_) {
        pos_ = static_cast<size_t>(pos);
    }
    else {
        throw eckit::SeriousBug("Failed to seek to position in file", Here());
    }
}


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
}  // namespace metkit::mars
