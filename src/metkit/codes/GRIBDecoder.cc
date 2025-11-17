/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "eckit/config/Resource.h"
#include "eckit/io/Buffer.h"
#include "eckit/message/Message.h"
#include "eckit/serialisation/MemoryStream.h"

#include "metkit/codes/GRIBDecoder.h"
#include "metkit/codes/api/CodesAPI.h"

#include <algorithm>
#include <iostream>

#include "eccodes.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

bool GRIBDecoder::match(const eckit::message::Message& msg) const {
    size_t len    = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and ((p[0] == 'G' and p[1] == 'R' and p[2] == 'I' and p[3] == 'B') or
                         (p[0] == 'T' and p[1] == 'I' and p[2] == 'D' and p[3] == 'E') or
                         (p[0] == 'B' and p[1] == 'U' and p[2] == 'D' and p[3] == 'G'));
}


void GRIBDecoder::getMetadata(const eckit::message::Message& msg, eckit::message::MetadataGatherer& gather,
                              const eckit::message::GetMetadataOptions& options) const {
    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");
    std::string nameSpace                     = options.nameSpace ? *options.nameSpace : gribToRequestNamespace;

    auto h = codesHandleFromMessage({static_cast<const uint8_t*>(msg.data()), msg.length()});

    for (auto& k : h->keys(nameSpace)) {
        auto name = k.name();

        if (name[0] == '_')
            continue;  // skip silly underscores in GRIB

        /* get key size to see if it is an array */
        if (h->size(name) != 1) {
            continue;
        }

        switch (options.valueRepresentation) {
            case eckit::message::ValueRepresentation::String: {
                gather.setValue(name, k.getString());
                break;
            }
            case eckit::message::ValueRepresentation::Native: {
                // https://jira.ecmwf.int/browse/ECC-2166
                if (name == "uuidOfHGrid") {
                    // uuidOfHGrid returns size 1 although it contains 16 bytes
                    gather.setValue(name, k.getString());
                }
                else {
                    std::visit(
                        [&](auto&& v) {
                            using Type = std::decay_t<decltype(v)>;
                            if constexpr (std::is_same_v<Type, std::string> || std::is_arithmetic_v<Type>) {
                                gather.setValue(name, std::forward<decltype(v)>(v));
                            }
                        },
                        k.get());
                }
                break;
            }
        }
    }

    // Explicit override for param (kludge for paramID handling)
    if (h->has("paramId")) {
        gather.setValue("param", h->getString("paramId"));
    }

    // Look for request embbeded in GRIB message
    if (h->has("localDefinitionNumber") && h->getLong("localDefinitionNumber") == 191) {
        /* TODO: Not grib2 compatible, but speed-up process */
        if (h->has("freeFormData")) {
            auto buffer = h->getBytes("freeFormData");

            eckit::MemoryStream s(buffer.data(), buffer.size());

            int count;
            s >> count;  // Number of requests
            ASSERT(count == 1);
            std::string tmp;
            s >> tmp;  // verb
            s >> count;
            for (int i = 0; i < count; i++) {
                std::string keyword, value;
                int n;
                s >> keyword;
                std::transform(keyword.begin(), keyword.end(), keyword.begin(), tolower);
                s >> n;  // Number of values
                ASSERT(n == 1);
                s >> value;
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                gather.setValue(keyword, value);
            }
        }
    }
}

eckit::Buffer GRIBDecoder::decode(const eckit::message::Message& msg) const {
    std::size_t size = msg.getSize("values");
    eckit::Buffer buf(size * sizeof(double));

    msg.getDoubleArray("values", reinterpret_cast<double*>(buf.data()), size);
    return buf;
}


void GRIBDecoder::print(std::ostream& s) const {
    s << "GRIBDecoder[]";
}

//----------------------------------------------------------------------------------------------------------------------

static GRIBDecoder gribDecoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
