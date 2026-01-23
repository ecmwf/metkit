/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <cstring>
#include <type_traits>

#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "eckit/parser/YAMLParser.h"

#include "metkit/codes/BUFRDecoder.h"
#include "metkit/codes/api/CodesAPI.h"
#include "metkit/codes/api/CodesTypes.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsRequest.h"

#include "eckit/exception/Exceptions.h"

#include "eccodes.h"

namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------

static std::map<long, long> subtypes_;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void readTable() {
    eckit::PathName bufrSubtypesPath =
        eckit::Resource<eckit::PathName>("bufrSubtypesPath;$BUFR_SUBTYPES_PATH", LibMetkit::bufrSubtypesYamlFile());

    const eckit::Value bufrSubtypes = eckit::YAMLParser::decodeFile(bufrSubtypesPath);
    const eckit::Value subtypes     = bufrSubtypes["subtypes"];
    ASSERT(subtypes.isList());
    for (size_t i = 0; i < subtypes.size(); ++i) {
        const eckit::Value s = subtypes[i];
        ASSERT(s.isList());
        ASSERT(s.size() == 2);
        subtypes_[s[0]] = s[1];
    }
}

bool BUFRDecoder::typeBySubtype(long subtype, long& type) {
    pthread_once(&once, readTable);

    auto s = subtypes_.find(subtype);
    if (s != subtypes_.end()) {
        type = s->second;
        return true;
    }
    return false;
}

//----------------------------------------------------------------------------------------------------------------------

bool BUFRDecoder::match(const eckit::message::Message& msg) const {
    size_t len    = msg.length();
    const char* p = static_cast<const char*>(msg.data());
    return len >= 4 and p[0] == 'B' and p[1] == 'U' and p[2] == 'F' and p[3] == 'R';
}


//----------------------------------------------------------------------------------------------------------------------


void BUFRDecoder::getMetadata(const eckit::message::Message& msg, eckit::message::MetadataGatherer& gather,
                              const eckit::message::GetMetadataOptions& options) const {

    auto h(codesHandleFromMessage({static_cast<const uint8_t*>(msg.data()), msg.length()}));

    /*
    // BUFR Performance improvement:
    https://confluence.ecmwf.int/display/UDOC/Performance+improvement+by+skipping+some+keys+-+ecCodes+BUFR+FAQ if
    ((unsigned long)(options.filter & eckit::message::MetadataFilter::IncludeExtraKeyAttributes) != 0) {
        CODES_CHECK(codes_set_long(h.get(), "skipExtraKeyAttributes", 1), 0);
    }
     */

    // we need to instruct ecCodes to unpack the data values:
    // https://confluence.ecmwf.int/display/ECC/bufr_keys_iterator
    h->set("unpack", 1);

    for (auto& k : h->keys()) {
        auto name = k.name();

        if (name == "subsetNumber") {
            continue;
        }

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
                std::visit(
                    [&](auto&& v) {
                        using Type = std::decay_t<decltype(v)>;
                        if constexpr (std::is_same_v<Type, std::string> || std::is_arithmetic_v<Type>) {
                            gather.setValue(name, std::forward<decltype(v)>(v));
                        }
                        else if constexpr (std::is_same_v<Type, std::vector<uint8_t>>) {
                            gather.setValue(name, k.getString());
                        }
                        else {
                            // Unhandled types are all array types - the prior call checking `size != 1` only allows for
                            // scalars.
                            throw eckit::Exception(
                                std::string("Unexpected type when accessing BURF message metadata ") + typeid(v).name(),
                                Here());
                        }
                    },
                    k.get());
                break;
            }
        }
    }
}


//----------------------------------------------------------------------------------------------------------------------

eckit::Buffer BUFRDecoder::decode(const eckit::message::Message& msg) const {
    std::size_t size = msg.getSize("numericValues");
    eckit::Buffer buf(size * sizeof(double));

    msg.getDoubleArray("numericValues", reinterpret_cast<double*>(buf.data()), size);
    return buf;
}


//----------------------------------------------------------------------------------------------------------------------

void BUFRDecoder::print(std::ostream& s) const {
    s << "BUFRDecoder[]";
}


//----------------------------------------------------------------------------------------------------------------------

static BUFRDecoder bufrDecoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
