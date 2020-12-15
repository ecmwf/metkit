/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @date   Jun 2020

#ifndef metkit_OdbMetadataDecoder_h
#define metkit_OdbMetadataDecoder_h

#include "eckit/message/Message.h"
#include "eckit/config/Resource.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/message/Message.h"
#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/io/DataHandle.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/odb/IdMapper.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypesFactory.h"

#include "odc/api/Odb.h"


namespace metkit {
namespace codes {

//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------

class OdbMetadataDecoder : public odc::api::SpanVisitor {

public:
    static std::vector<std::string>& columnNames();

    OdbMetadataDecoder(eckit::message::MetadataGatherer& gatherer, const std::string& verb = "retrieve");

    virtual void operator()(const std::string& columnName, const std::set<long>& vals);
    virtual void operator()(const std::string& columnName, const std::set<double>& vals);
    virtual void operator()(const std::string& columnName, const std::set<std::string>& vals);

private: // methods

    template <typename T>
    void visit(const std::string& columnName, const std::set<T>& vals, const metkit::mars::MarsLanguage& language) {

        auto mapitr = mapping_.find(columnName);
        ASSERT(mapitr != mapping_.end());
        std::string keyword = eckit::StringTools::lower(mapitr->second);
        metkit::mars::Type* t = language.type(keyword);

        for(auto val: vals) {
            std::string stringVal = eckit::Translator<T, std::string>()(val);
            std::string tidyVal = t->tidy(stringVal);
            if (stringVal == tidyVal) // if t->tidy had no effect, set the original value
                gather_.setValue(keyword, val);
            else
                gather_.setValue(keyword, tidyVal);
        }
    }

private:
    static void init();

private: // members

    static std::map<std::string, std::string> mapping_;
    static std::vector<std::string> columnNames_;

    metkit::mars::MarsLanguage language_;
    eckit::message::MetadataGatherer& gather_;

};


//----------------------------------------------------------------------------------------------------------------------

} // namespace codes
} // namespace metkit

#endif
