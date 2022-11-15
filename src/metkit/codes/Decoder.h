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

#include "eccodes.h"
#include "eckit/message/Decoder.h"
#include "eckit/message/Message.h"

namespace metkit {
namespace codes {
namespace decoder {

template <typename T>
class HandleDeleter {
    T* h_;

public:
    HandleDeleter(T* h) :
        h_(h) {}
    ~HandleDeleter() noexcept(true);

    T* get();
};


// Maps specific flags to Eccodes flag.
// TODO make constexpr in C++14
unsigned long metadataFilterFlagToEccodes(eckit::message::MetadataFilter f) noexcept;
// Maps a combined flags to an eccodes
// TODO make constexpr in C++14
unsigned long metadataFilterToEccodes(eckit::message::MetadataFilter f) noexcept;


template <typename ItCtx, typename GetString>
bool decodeString(codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* itCtx, const std::string& name, GetString&& getString) {
    char val[1024];
    size_t len = sizeof(val);
    ASSERT(getString(h, itCtx, name, val, &len) == 0);
    if (*val) {
        gather.setValue(name, val);
        return true;
    }
    return false;
}

template <typename ItCtx, typename GetLong>
bool decodeLong(codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* itCtx, const std::string& name, GetLong&& getLong) {
    long l;
    size_t len = 1;
    if (getLong(h, itCtx, name, &l, &len) == 0) {
        gather.setValue(name, l);
        return true;
    }
    return false;
}

template <typename ItCtx, typename GetDouble>
bool decodeDouble(codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* itCtx, const std::string& name, GetDouble&& getDouble) {
    double d;
    size_t len = 1;
    if (getDouble(h, itCtx, name, &d, &len) == 0) {
        gather.setValue(name, d);
        return true;
    }
    return false;
}

template <typename ItCtx, typename GetBytes, typename GetString>
bool decodeByte(codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* itCtx, const std::string& name, GetBytes&& getBytes, GetString&& getString) {
    // TODO the field uuidOfHGrid is of native type BYTE and returns 1 for codes_get_size,
    // however eccodes prints an error because it requires 16bytes and should probably be decoded as string
    unsigned char c[1024];
    size_t len = sizeof(c);
    if (getBytes(h, itCtx, name, c, &len) == 0) {
        if (len == 1) {
            gather.setValue(name, static_cast<long>(c[0]));
            return true;
        }
        else {
            // Decoded a UUID ... convert to string
            return decodeString(h, gather, itCtx, name, getString);
        }
    }
    return false;
}

template <typename ItCtx, typename GetString, typename GetLong, typename GetDouble, typename GetBytes>
bool decodeNative(codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* itCtx, const std::string& name, GetString&& getString, GetLong&& getLong, GetDouble&& getDouble, GetBytes&& getBytes) {
    int keyType = 0;
    ASSERT(codes_get_native_type(h, name.c_str(), &keyType) == 0);
    // GRIB_ Type prefixes are also valid for BUFR
    switch (keyType) {
        case GRIB_TYPE_LONG: {
            return decodeLong(h, gather, itCtx, name, getLong);
        }
        case GRIB_TYPE_DOUBLE: {
            return decodeDouble(h, gather, itCtx, name, getDouble);
        }
        case GRIB_TYPE_STRING: {
            return decodeString(h, gather, itCtx, name, getString);
        }
        case GRIB_TYPE_BYTES: {
            return decodeByte(h, gather, itCtx, name, getBytes, getString);
        }
        default: {
            // String decoding should be always possible
            return decodeString(h, gather, itCtx, name, getString);
        }
    }
    return true;
}

template <typename ItCtx, typename ItNextName, typename DecodeFunc>
void iterateMetadata(codes_handle* h, ItCtx* itCtx, eckit::message::MetadataGatherer& gather, ItNextName&& itNextName, DecodeFunc&& decodeFunc) {
    eckit::Optional<std::string> name;
    while ((name = itNextName(h, itCtx))) {
        size_t klen = 0;

        /* get key size to see if it is an array */
        ASSERT(codes_get_size(h, name->c_str(), &klen) == 0);
        if (klen != 1) {
            continue;
        }
        decodeFunc(h, gather, itCtx, *name);
    }
}

template <typename InitIt, typename ItNextName, typename GetString, typename GetLong, typename GetDouble, typename GetBytes, typename PostProcess>
void getMetadata(const eckit::message::Message& msg, eckit::message::MetadataGatherer& gather, const eckit::message::GetMetadataOptions& options, InitIt&& initIt, ItNextName&& itNextName, GetString&& getString, GetLong&& getLong, GetDouble&& getDouble, GetBytes&& getBytes, PostProcess&& postProcess) {
    codes_handle* h = codes_handle_new_from_message(nullptr, msg.data(), msg.length());
    ASSERT(h);
    HandleDeleter<codes_handle> handleDeleter(h);

    auto itCtx  = initIt(h);
    ASSERT(itCtx);
    using ItCtx = typename std::remove_pointer<decltype(itCtx)>::type;
    HandleDeleter<ItCtx> itDeleter(itCtx);

    switch (options.valueRepresentation) {
        case eckit::message::ValueRepresentation::Native:
            iterateMetadata(h, itCtx, gather, std::forward<ItNextName>(itNextName),
                            [&getString, &getLong, &getDouble, &getBytes](codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* itCtx, const std::string& name) {
                                decodeNative(h, gather, itCtx, name, getString, getLong, getDouble, getBytes);
                            });
            break;
        case eckit::message::ValueRepresentation::String:
            iterateMetadata(h, itCtx, gather, std::forward<ItNextName>(itNextName),
                            [&getString](codes_handle* h, eckit::message::MetadataGatherer& gather, ItCtx* itCtx, const std::string& name) {
                                decodeString(h, gather, itCtx, name, getString);
                            });
            break;
    }

    postProcess(h, gather, itCtx);
}


}  // namespace decoder
}  // namespace codes
}  // namespace metkit
