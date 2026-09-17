/*
 * (C) Copyright 2026- ECMWF and individual contributors.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0.
 */

#include "metkit/mars2grib/testing-utils/Producer.h"

#include <cstdio>
#include <utility>
#include <vector>

#include <zstd.h>

#include "eckit/exception/Exceptions.h"
#include "metkit/mars2grib/utils/profiling/Profiling.h"

namespace metkit::mars2grib::testing_utils::run_tests {

class Producer::Impl {
public:

    template <class Cntx_t>
    explicit Impl(const std::string& archivePath, Cntx_t& cntx) : archivePath_{archivePath} {
        utils::profiling::profileEnterFunction(cntx, Here());
        file_ = std::fopen(archivePath.c_str(), "rb");
        if (file_ == nullptr) {
            throw eckit::UserError("Unable to open test-case archive `" + archivePath + "`", Here());
        }

        stream_ = ZSTD_createDStream();
        if (stream_ == nullptr) {
            std::fclose(file_);
            file_ = nullptr;
            throw eckit::Exception("Unable to create zstd decompression stream", Here());
        }

        std::size_t resultOpt = ZSTD_DCtx_setParameter(stream_, ZSTD_d_windowLogMax, 29);

        if (ZSTD_isError(resultOpt)) {
            ZSTD_freeDStream(stream_);
            stream_ = nullptr;
            std::fclose(file_);
            file_ = nullptr;

            throw eckit::Exception(
                "Unable to configure zstd decompression stream: " + std::string{ZSTD_getErrorName(resultOpt)}, Here());
        }

        const std::size_t result = ZSTD_initDStream(stream_);
        if (ZSTD_isError(result)) {
            ZSTD_freeDStream(stream_);
            stream_ = nullptr;
            std::fclose(file_);
            file_ = nullptr;
            throw eckit::Exception(
                "Unable to initialise zstd decompression stream: " + std::string{ZSTD_getErrorName(result)}, Here());
        }

        inputStorage_.resize(ZSTD_DStreamInSize());
        outputStorage_.resize(ZSTD_DStreamOutSize());
        utils::profiling::profileExitFunction(cntx, Here());
    }

    ~Impl() {
        if (stream_ != nullptr) {
            ZSTD_freeDStream(stream_);
        }
        if (file_ != nullptr) {
            std::fclose(file_);
        }
    }

    template <class Cntx_t>
    std::optional<std::string> readNextRecord(Cntx_t& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        while (true) {
            const std::size_t newline = pending_.find('\n', pendingOffset_);
            if (newline != std::string::npos) {
                std::optional<std::string> result =
                    pending_.substr(pendingOffset_, newline - pendingOffset_);
                pendingOffset_     = newline + 1;
                if (!result->empty() && result->back() == '\r') {
                    result->pop_back();
                }
                utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }

            if (inputFinished_) {
                if (pendingOffset_ == pending_.size()) {
                    std::optional<std::string> result = std::nullopt;
                    utils::profiling::profileExitFunction(cntx, Here());
                    return result;
                }
                std::optional<std::string> result = pending_.substr(pendingOffset_);
                pending_.clear();
                pendingOffset_ = 0;
                if (!result->empty() && result->back() == '\r') {
                    result->pop_back();
                }
                utils::profiling::profileExitFunction(cntx, Here());
                return result;
            }

            decompressMore(utils::profiling::callSite(cntx, Here()));
        }
    }

private:

    template <class Cntx_t>
    void decompressMore(Cntx_t& cntx) {
        utils::profiling::profileEnterFunction(cntx, Here());
        if (pendingOffset_ != 0) {
            pending_.erase(0, pendingOffset_);
            pendingOffset_ = 0;
        }

        if (input_.pos == input_.size) {
            const std::size_t bytesRead = std::fread(inputStorage_.data(), 1, inputStorage_.size(), file_);
            if (bytesRead == 0) {
                if (std::ferror(file_) != 0) {
                    throw eckit::Exception("Error while reading test-case archive `" + archivePath_ + "`", Here());
                }
                if (started_ && remaining_ != 0) {
                    throw eckit::Exception("Truncated zstd test-case archive `" + archivePath_ + "`", Here());
                }
                inputFinished_ = true;
                utils::profiling::profileExitFunction(cntx, Here());
                return;
            }
            input_ = ZSTD_inBuffer{inputStorage_.data(), bytesRead, 0};
        }

        ZSTD_outBuffer output{outputStorage_.data(), outputStorage_.size(), 0};
        remaining_ = ZSTD_decompressStream(stream_, &output, &input_);
        started_   = true;
        if (ZSTD_isError(remaining_)) {
            throw eckit::Exception("Unable to decompress test-case archive `" + archivePath_ +
                                       "`: " + std::string{ZSTD_getErrorName(remaining_)},
                                   Here());
        }
        pending_.append(outputStorage_.data(), output.pos);
        utils::profiling::profileExitFunction(cntx, Here());
    }

    std::string archivePath_;
    std::FILE* file_{nullptr};
    ZSTD_DStream* stream_{nullptr};
    std::vector<char> inputStorage_;
    std::vector<char> outputStorage_;
    ZSTD_inBuffer input_{nullptr, 0, 0};
    std::string pending_;
    std::size_t pendingOffset_{0};
    std::size_t remaining_{0};
    bool started_{false};
    bool inputFinished_{false};
};

Producer::Producer(const std::string& archivePath) {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    impl_ = std::make_unique<Impl>(archivePath, utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
}

Producer::~Producer() = default;

std::optional<std::string> Producer::readNextRecord() {
    utils::profiling::NoProfileContext cntx;
    utils::profiling::profileEnterFunction(cntx, Here());
    std::optional<std::string> result = impl_->readNextRecord(utils::profiling::callSite(cntx, Here()));
    utils::profiling::profileExitFunction(cntx, Here());
    return result;
}

}  // namespace metkit::mars2grib::testing_utils::run_tests
