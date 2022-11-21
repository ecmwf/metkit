/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Philipp Geier
/// @date   Nov 2022

#pragma once

#include "eckit/message/Decoder.h"
#include "eckit/message/Message.h"

typedef struct grib_handle codes_handle;

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

template <typename T>
class HandleDeleter {
    T* h_;

public:
    HandleDeleter(T* h) :
        h_(h) {}
    ~HandleDeleter();

    T* get();
};

//----------------------------------------------------------------------------------------------------------------------


// Maps specific flags to Eccodes flag.
// TODO make constexpr in C++14
unsigned long metadataFilterFlagToEccodes(eckit::message::MetadataFilter f);
// Maps a combined flags to an eccodes
// TODO make constexpr in C++14
unsigned long metadataFilterToEccodes(eckit::message::MetadataFilter f);


//----------------------------------------------------------------------------------------------------------------------

template <typename GetString>
bool decodeString(codes_handle* h,
                  eckit::message::MetadataGatherer& gather,
                  const char* name,
                  GetString&& getString) {
    char val[1024];
    size_t len = sizeof(val);
    ASSERT(getString(h,
                     name,
                     val,
                     &len)
           == 0);
    if (*val) {
        gather.setValue(name,
                        val);
        return true;
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------

template <typename GetLong>
bool decodeLong(codes_handle* h,
                eckit::message::MetadataGatherer& gather,
                const char* name,
                GetLong&& getLong) {
    long l;
    size_t len = 1;
    if (getLong(h,
                name,
                &l,
                &len)
        == 0) {
        gather.setValue(name,
                        l);
        return true;
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------

template <typename GetDouble>
bool decodeDouble(codes_handle* h,
                  eckit::message::MetadataGatherer& gather,
                  const char* name,
                  GetDouble&& getDouble) {
    double d;
    size_t len = 1;
    if (getDouble(h, name, &d, &len) == 0) {
        gather.setValue(name, d);
        return true;
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------

template <typename GetBytes,
          typename GetString>
bool decodeByte(codes_handle* h,
                eckit::message::MetadataGatherer& gather,
                const char* name,
                GetBytes&& getBytes,
                GetString&& getString) {
    // TODO the field uuidOfHGrid is of native type BYTE and returns 1 for codes_get_size,
    // however eccodes prints an error because it requires 16bytes and should probably be decoded as string
    unsigned char c[1024];
    size_t len = sizeof(c);
    if (getBytes(h, name, c, &len) == 0) {
        if (len == 1) {
            gather.setValue(name, static_cast<long>(c[0]));
            return true;
        }
        else {
            // Decoded a UUID ... convert to string
            return decodeString(h, gather, name, getString);
        }
    }
    return false;
}


//----------------------------------------------------------------------------------------------------------------------

enum class NativeType : unsigned
{
    Unknown = 0,
    String  = 1,
    Long    = 2,
    Double  = 3,
    Bytes   = 4,
};


NativeType getNativeType(codes_handle* h, const char* name);


template <typename GetString,
          typename GetLong,
          typename GetDouble,
          typename GetBytes>
bool decodeNative(codes_handle* h,
                  eckit::message::MetadataGatherer& gather,
                  const char* name,
                  GetString&& getString,
                  GetLong&& getLong,
                  GetDouble&& getDouble,
                  GetBytes&& getBytes) {
    switch (getNativeType(h, name)) {
        case NativeType::Long: {
            return decodeLong(h, gather, name, getLong);
        }
        case NativeType::Double: {
            return decodeDouble(h, gather, name, getDouble);
        }
        case NativeType::String: {
            return decodeString(h, gather, name, getString);
        }
        case NativeType::Bytes: {
            return decodeByte(h, gather, name, getBytes, getString);
        }
        default: {
            // String decoding should be always possible
            return decodeString(h, gather, name, getString);
        }
    }
    return true;
}


//----------------------------------------------------------------------------------------------------------------------

template <typename GetString,
          typename GetLong,
          typename GetDouble,
          typename GetBytes,
          typename ForwardToFunc>
void withSpecializedDecoder(
    const eckit::message::GetMetadataOptions& options,
    GetString&& getString,
    GetLong&& getLong,
    GetDouble&& getDouble,
    GetBytes&& getBytes,
    ForwardToFunc&& func) {

    switch (options.valueRepresentation) {
        case eckit::message::ValueRepresentation::String:
            std::forward<ForwardToFunc>(func)(
                [&getString](codes_handle* h,
                             eckit::message::MetadataGatherer& gather,
                             const char* name) {
                    decodeString(h, gather, name, getString);
                });
            return;
        default:
            std::forward<ForwardToFunc>(func)(
                [&getString,
                 &getLong,
                 &getDouble,
                 &getBytes](codes_handle* h,
                            eckit::message::MetadataGatherer& gather,
                            const char* name) {
                    decodeNative(h,
                                 gather,
                                 name,
                                 getString,
                                 getLong,
                                 getDouble,
                                 getBytes);
                });
            return;
    }
}


}  // namespace codes
}  // namespace metkit
