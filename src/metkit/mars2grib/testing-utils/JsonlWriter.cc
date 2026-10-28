/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/JsonlWriter.h"

#include <cstdio>
#include <vector>

#include <zstd.h>

#include "eckit/exception/Exceptions.h"

#include "metkit/mars2grib/testing-utils/Producer.h"

namespace metkit::mars2grib::testing_utils::run_tests {

namespace {

constexpr std::size_t flushThreshold = 1 << 20;

}  // namespace

class JsonlWriter::Impl {
public:

    Impl(const std::string& path, int zstdLevel) : path_{path}, compressed_{isZstdPath(path)} {
        file_ = std::fopen(path.c_str(), "wb");
        if (file_ == nullptr) {
            throw eckit::UserError("Unable to open output file `" + path + "`", Here());
        }

        if (compressed_) {
            context_ = ZSTD_createCCtx();
            if (context_ == nullptr) {
                release();
                throw eckit::Exception("Unable to create zstd compression context", Here());
            }
            try {
                check(ZSTD_CCtx_setParameter(context_, ZSTD_c_compressionLevel, zstdLevel),
                      "set zstd compression level");
                check(ZSTD_CCtx_setParameter(context_, ZSTD_c_checksumFlag, 1), "enable zstd checksum");
            }
            catch (...) {
                release();
                throw;
            }
            outputStorage_.resize(ZSTD_CStreamOutSize());
        }

        buffer_.reserve(2 * flushThreshold);
    }

    ~Impl() {
        if (!closed_) {
            try {
                close();
            }
            catch (...) {
            }
        }
        release();
    }

    void write(std::string_view record) {
        if (closed_) {
            throw eckit::Exception("Write to closed output file `" + path_ + "`", Here());
        }
        buffer_.append(record.data(), record.size());
        buffer_.push_back('\n');
        if (buffer_.size() >= flushThreshold) {
            flush(false);
        }
    }

    void close() {
        if (closed_) {
            return;
        }
        closed_ = true;
        flush(true);
        if (std::fflush(file_) != 0) {
            throw eckit::Exception("Unable to flush output file `" + path_ + "`", Here());
        }
        const int status = std::fclose(file_);
        file_            = nullptr;
        if (status != 0) {
            throw eckit::Exception("Unable to close output file `" + path_ + "`", Here());
        }
    }

private:

    void check(std::size_t code, const char* what) const {
        if (ZSTD_isError(code)) {
            throw eckit::Exception(
                "Unable to " + std::string{what} + " for `" + path_ + "`: " + std::string{ZSTD_getErrorName(code)},
                Here());
        }
    }

    void writeRaw(const char* data, std::size_t size) {
        if (size != 0 && std::fwrite(data, 1, size, file_) != size) {
            throw eckit::Exception("Unable to write output file `" + path_ + "`", Here());
        }
    }

    void flush(bool finish) {
        if (!compressed_) {
            writeRaw(buffer_.data(), buffer_.size());
            buffer_.clear();
            return;
        }

        const ZSTD_EndDirective mode = finish ? ZSTD_e_end : ZSTD_e_continue;
        ZSTD_inBuffer input{buffer_.data(), buffer_.size(), 0};
        while (true) {
            ZSTD_outBuffer output{outputStorage_.data(), outputStorage_.size(), 0};
            const std::size_t remaining = ZSTD_compressStream2(context_, &output, &input, mode);
            check(remaining, "compress");
            writeRaw(outputStorage_.data(), output.pos);
            const bool done = finish ? remaining == 0 : input.pos == input.size;
            if (done) {
                break;
            }
        }
        buffer_.clear();
    }

    void release() {
        if (context_ != nullptr) {
            ZSTD_freeCCtx(context_);
            context_ = nullptr;
        }
        if (file_ != nullptr) {
            std::fclose(file_);
            file_ = nullptr;
        }
    }

    std::string path_;
    bool compressed_;
    std::FILE* file_{nullptr};
    ZSTD_CCtx* context_{nullptr};
    std::vector<char> outputStorage_;
    std::string buffer_;
    bool closed_{false};
};

JsonlWriter::JsonlWriter(const std::string& path, int zstdLevel) : impl_{std::make_unique<Impl>(path, zstdLevel)} {}

JsonlWriter::~JsonlWriter() = default;

void JsonlWriter::write(std::string_view record) {
    impl_->write(record);
}

void JsonlWriter::close() {
    impl_->close();
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
