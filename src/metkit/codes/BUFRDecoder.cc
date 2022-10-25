/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "eccodes.h"

#include "metkit/codes/BUFRDecoder.h"

#include "eckit/config/Resource.h"
#include "eckit/message/Message.h"
#include "eckit/parser/YAMLParser.h"

#include "metkit/config/LibMetkit.h"
#include "metkit/mars/MarsRequest.h"

namespace metkit {
namespace codes {

namespace {
class HandleDeleter {
    codes_handle *h_;
public:
    HandleDeleter(codes_handle *h) : h_(h) {}
    ~HandleDeleter() {
        if (h_) {
            codes_handle_delete(h_);
        }
    }

    codes_handle* get() { return h_; }

};
}

//----------------------------------------------------------------------------------------------------------------------

static std::map<long, long> subtypes_;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void readTable()
{
    eckit::PathName bufrSubtypesPath = eckit::Resource<eckit::PathName>("bufrSubtypesPath;$BUFR_SUBTYPES_PATH", LibMetkit::bufrSubtypesYamlFile());

    const eckit::Value bufrSubtypes = eckit::YAMLParser::decodeFile(bufrSubtypesPath);
    const eckit::Value subtypes = bufrSubtypes["subtypes"];
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


void BUFRDecoder::getMetadata(const eckit::message::Message& msg,
                              eckit::message::MetadataGatherer& gather) const  {
    // This function has been implemented similarly to the GRIBDecoder and following 
    // https://confluence.ecmwf.int/display/ECC/bufr_keys_iterator
    // It has been possible to extract flat metadata with multio-feed. However someone with more profound knowledge on BUFR and ECCODES may have improvements.
    
    /// @TODO Do we need something like this? Compare with GRIBDecoder...
    // static std::string bufrToRequestNamespace = eckit::Resource<std::string>("bufrToRequestNamespace", "mars");


    codes_handle* h = codes_handle_new_from_message(nullptr, msg.data(), msg.length());
    ASSERT(h);
    HandleDeleter d(h);
    
    /* we need to instruct ecCodes to unpack the data values: https://confluence.ecmwf.int/display/ECC/bufr_keys_iterator */
    CODES_CHECK(codes_set_long(h, "unpack", 1), 0);

    codes_bufr_keys_iterator *ks = codes_bufr_keys_iterator_new(h,
                             CODES_KEYS_ITERATOR_ALL_KEYS);

    ASSERT(ks);

    while (codes_bufr_keys_iterator_next(ks)) {
        const char *name = codes_bufr_keys_iterator_get_name(ks);

      
        if (std::string("subsetNumber") == name) {
                continue;
        }

        int keyType = 0; // @TODO Extracting the keyType is not required but may be helpful to call the right get overload directly. However I can not find any documentation on types
        size_t klen = 0;
        char val[1024];
        size_t len = sizeof(val);
        double d;
        long l;
        
        ASSERT(codes_get_native_type(h, name, &keyType) == 0);

        /* get key size to see if it is an array */
        ASSERT(codes_get_size(h, name, &klen) == 0);
        
        if (klen != 1) {
            continue;
        }
        

        ASSERT( codes_get_string(h, name, val, &len) == 0);

        if (*val) {
            gather.setValue(name, val);
        }

        len = 1;
        if (codes_get_double(h, name, &d) == 0)       {
            gather.setValue(name, d);
        }
        len = 1;
        if (codes_get_long(h, name, &l) == 0)         {
            gather.setValue(name, l);
        }

    }

    codes_bufr_keys_iterator_delete(ks);
}


eckit::Buffer BUFRDecoder::decode(const eckit::message::Message& msg) const  {
    std::vector<double> v;
    // @TODO Is this valid for all BUFR messages? Worked with results from mars
    msg.getDoubleArray("numericValues", v);
    
    return eckit::Buffer(reinterpret_cast<void *>(v.data()), v.size()*sizeof(double));
}


void BUFRDecoder::print(std::ostream& s) const {
    s << "BUFRDecoder[]";
}


static BUFRDecoder decoder;

//----------------------------------------------------------------------------------------------------------------------

}  // namespace codes
}  // namespace metkit
