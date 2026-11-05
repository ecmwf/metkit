// SPDX-FileCopyrightText: 2017- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "eckit/io/Offset.h"
#include "metkit/codes/api/CodesAPI.h"

#include "eckit/message/MessageContent.h"

namespace metkit::codes {

//----------------------------------------------------------------------------------------------------------------------

class CodesDataContent : public eckit::message::MessageContent {
public:

    CodesDataContent(std::unique_ptr<CodesHandle> handle, eckit::Offset offset);
    CodesDataContent(std::unique_ptr<CodesHandle> handle);

    virtual ~CodesDataContent() = default;

    const CodesHandle& codesHandle() const;
    CodesHandle& codesHandle();

protected:

    std::unique_ptr<CodesHandle> handle_;
    eckit::Offset offset_;

    using eckit::message::MessageContent::transform;
    void transform(const eckit::OrderedStringDict&) override;

private:

    size_t length() const override;
    void write(eckit::DataHandle& handle) const override;
    eckit::DataHandle* readHandle() const override;
    void print(std::ostream& s) const override;
    std::string getString(const std::string& key) const override;
    long getLong(const std::string& key) const override;
    double getDouble(const std::string& key) const override;
    void getDoubleArray(const std::string& key, std::vector<double>& values) const override;
    void getFloatArray(const std::string& key, std::vector<float>& values) const override;
    size_t getSize(const std::string& key) const override;
    void getDoubleArray(const std::string& key, double* data, size_t lenExpected) const override;
    void getFloatArray(const std::string& key, float* data, size_t lenExpected) const override;

    eckit::Offset offset() const override;
    const void* data() const override;
};


//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::codes
