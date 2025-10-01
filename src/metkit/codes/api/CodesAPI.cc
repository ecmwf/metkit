/*
 * (C) Copyright 2025- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/CodesTypes.h"

#include "eccodes.h"

#include <sstream>


namespace metkit::codes {


std::string samplesPath() {
    return codes_samples_path(NULL);
}

std::string definitionPath() {
    return codes_definition_path(NULL);
}

long getAPIVersion() {
    return codes_get_api_version();
}

std::string getGitSha1() {
    return codes_get_git_sha1();
}

std::string getGitBranch() {
    return codes_get_git_branch();
}

std::string getBuildDate() {
    return codes_get_build_date();
}

std::string getPackageName() {
    return codes_get_package_name();
}


std::string info() {
    std::ostringstream oss;
    oss << "eccodes{";
    oss << "api-version: " << getAPIVersion();
    oss << ", git-sha1: " << getGitSha1();
    oss << ", git-branch: " << getGitBranch();
    oss << ", build-date: " << getBuildDate();
    oss << ", package-name: " << getPackageName();
    oss << "}";
    return oss.str();
}


void checkCodes(int code) {
    if (code != 0) {
        std::string msg(codes_get_error_message(code));
        throw CodesException(msg);
    }
};

template <typename T>
CodesHandlePtr* castFromCodes(T* ptr) {
    return static_cast<CodesHandlePtr*>(static_cast<void*>(ptr));
}


[[nodiscard]]
CodesHandlePtr* newFromMessage(Span<const unsigned char> data) {
    return castFromCodes(codes_handle_new_from_message_copy(NULL, static_cast<const void*>(data.data()), data.size()));
}

[[nodiscard]]
CodesHandlePtr* newFromSample(std::string sampleName, std::optional<Product> product) {
    if (product) {
        switch (*product) {
            case Product::GRIB:
                return castFromCodes(codes_grib_handle_new_from_samples(NULL, sampleName.c_str()));
            case Product::BUFR:
                return castFromCodes(codes_bufr_handle_new_from_samples(NULL, sampleName.c_str()));
            default:
                return castFromCodes(codes_handle_new_from_samples(NULL, sampleName.c_str()));
        }
    }
    return castFromCodes(codes_handle_new_from_samples(NULL, sampleName.c_str()));
}

[[nodiscard]]
CodesHandlePtr* newFromFile(FILE* file, Product product) {
    int err             = 0;
    CodesHandlePtr* ret = nullptr;
    switch (product) {
        case Product::GRIB:
            ret = castFromCodes(codes_grib_handle_new_from_file(NULL, file, &err));
            break;
        case Product::BUFR:
            ret = castFromCodes(codes_bufr_handle_new_from_file(NULL, file, &err));
            break;
    };
    checkCodes(err);
    if (ret == nullptr) {
        throw CodesException("codes_handle_new_from_file returned NULL without an additional error");
    }

    return ret;
}

}  // namespace metkit::codes
