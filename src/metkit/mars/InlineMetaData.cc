/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars/InlineMetaData.h"
#include "eckit/types/Types.h"

using namespace eckit;

constexpr long kVersion = 1;

namespace metkit::mars {

template <typename MARS_REQUEST>
InlineMetaDataImpl<MARS_REQUEST>::InlineMetaDataImpl(const StringList& names, const std::vector<StringList>& values,
                               const Length& dataLength) :
    length_(dataLength) {
    request_.reset(new MARS_REQUEST("metadata"));
    ASSERT(names.size() == values.size());
    for (Ordinal i = 0; i < names.size(); i++) {
        if (names[i] != "")
            request_->values(names[i], values[i]);
    }
    Log::info() << *request_;
}

template <typename MARS_REQUEST>
InlineMetaDataImpl<MARS_REQUEST>::InlineMetaDataImpl(MARS_REQUEST* r, const Length& dataLength) : length_(dataLength), request_(r) {}


// NB: See comment in header file about serialisation.
template <typename MARS_REQUEST>
InlineMetaDataImpl<MARS_REQUEST>::InlineMetaDataImpl(eckit::Stream& s) {

    ASSERT(s.next());  // - begin InlineMetaData
   
    std::string klsName;
    s >> klsName;
    ASSERT(klsName == "InlineMetaData");

    long v;
    s >> v;
    if (v != kVersion) {
        ASSERT(v < kVersion);
        Log::warning() << "InlineMetaDataImpl: version mismatch" << std::endl;
    }
    s >> length_;

    ASSERT(s.next());  // -- begin marsrequest

    s >> klsName;
    ASSERT(klsName == "MarsRequest");

    request_.reset(new MARS_REQUEST(s));

    s.skipEndObject();  // -- end marsrequest

    s.skipEndObject(); // - end InlineMetaData
}

template <typename MARS_REQUEST>
InlineMetaDataImpl<MARS_REQUEST>::~InlineMetaDataImpl() {}

// NB: See comment in header file about serialisation.
template <typename MARS_REQUEST>
void InlineMetaDataImpl<MARS_REQUEST>::encode(eckit::Stream& s) const {
    // We are bypassing the reanimation of the MarsRequest object AND the InlineMetaDataImpl object.

    s.startObject(); // begin InlineMetaData
    s << "InlineMetaData";
    s << kVersion;
    s << length_;
    
    s.startObject(); // begin marsrequest
    s << "MarsRequest";
    request_->encode(s);
    s.endObject();  // end marsrequest
    
    s.endObject();  // end InlineMetaData
}

template <typename MARS_REQUEST>
void InlineMetaDataImpl<MARS_REQUEST>::print(std::ostream& s) const {
    s << "version " << kVersion << ", length " << length_ << " request " << *request_;
}

template class  InlineMetaDataImpl<MarsRequest>;

} // namespace metkit::mars

