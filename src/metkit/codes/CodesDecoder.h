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

#include <eccodes.h>


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

template <typename DERIVED>
class CodesDecoder : public eckit::message::MessageDecoder {

protected:

    void decodeKey(codes_handle* h, codes_keys_iterator* it, const char* name, eckit::message::MetadataGatherer& gather,
                   const eckit::message::GetMetadataOptions& options) const {

        if (options.valueRepresentation == eckit::message::ValueRepresentation::String) {
            decodeString(h, it, gather, name);
        }
        else {
            ASSERT(options.valueRepresentation == eckit::message::ValueRepresentation::Native);
            decodeNative(h, it, gather, name);
        }
    }

private:

    void decodeNative(codes_handle* h, codes_keys_iterator* it, eckit::message::MetadataGatherer& gather,
                      const char* name) const {

        int key_type = 0;
        ASSERT(codes_get_native_type(h, name, &key_type) == 0);

        // n.b. GRIB_ type prefixes valid for both GRIB and BUFR!!
        switch (key_type) {
            case GRIB_TYPE_LONG:
                decodeLong(h, it, gather, name);
                break;
            case GRIB_TYPE_DOUBLE:
                decodeDouble(h, it, gather, name);
                break;
            case GRIB_TYPE_STRING:
                decodeString(h, it, gather, name);
                break;
            case GRIB_TYPE_BYTES:
                decodeByte(h, it, gather, name);
                break;
            default:
                throw eckit::SeriousBug("Unrecognised codes key native type", Here());
        }
    }

    void decodeByte(codes_handle* h, codes_keys_iterator* it, eckit::message::MetadataGatherer& gather,
                    const char* name) const {
        // TODO the field uuidOfHGrid is of native type BYTE and returns 1 for codes_get_size,
        // however eccodes prints an error because it requires 16bytes and should probably be decoded as string
        unsigned char c[1024];
        size_t len = sizeof(c);
        if (static_cast<const DERIVED*>(this)->getBytes(h, it, name, c, &len)) {
            if (len == 1) {
                gather.setValue(name, static_cast<long>(c[0]));
            }
            else {
                // Decoded a UUID ... convert to string
                decodeString(h, it, gather, name);
            }
        }
    }

    void decodeString(codes_handle* h, codes_keys_iterator* it, eckit::message::MetadataGatherer& gather,
                      const char* name) const {
        gather.setValue(name, static_cast<const DERIVED*>(this)->getString(h, it, name));
    }

    void decodeLong(codes_handle* h, codes_keys_iterator* it, eckit::message::MetadataGatherer& gather,
                    const char* name) const {
        gather.setValue(name, static_cast<const DERIVED*>(this)->getLong(h, it, name));
    }

    void decodeDouble(codes_handle* h, codes_keys_iterator* it, eckit::message::MetadataGatherer& gather,
                      const char* name) const {
        gather.setValue(name, static_cast<const DERIVED*>(this)->getDouble(h, it, name));
    }
};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
