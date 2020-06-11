/*
 * (C) Copyright 1996-2012 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "metkit/odb/OdbToRequest.h"

#include <algorithm>

#include "eckit/config/Resource.h"
#include "eckit/config/YAMLConfiguration.h"
#include "eckit/io/DataHandle.h"
#include "eckit/types/Types.h"
#include "eckit/utils/StringTools.h"

#include "odc/api/Odb.h"

#include "metkit/mars/MarsLanguage.h"
#include "metkit/config/LibMetkit.h"
#include "metkit/odb/IdMapper.h"
#include "metkit/mars/Type.h"
#include "metkit/mars/TypesFactory.h"


using namespace eckit;
using namespace odc::api;

namespace metkit {
using namespace mars;

namespace odb {

//----------------------------------------------------------------------------------------------------------------------

class MarsRequestSetter : public SpanVisitor {
public:  // methods
    MarsRequestSetter(MarsRequest& request, MarsLanguage& lang, const StringDict& mapping) :
        request_(request), language_(lang), mapping_(mapping) {}

private:  // methods
    template <typename T>
    void visit(const std::string& columnName, const std::set<T>& vals) {
        LOG_DEBUG_LIB(LibMetkit) << "MarsRequestSetter::visit() columnName: " << columnName
                                 << " vals: " << vals << std::endl;

        auto mapitr = mapping_.find(columnName);
        ASSERT(mapitr != mapping_.end());
        std::vector<std::string> stringvals;
        stringvals.reserve(vals.size());
        std::transform(vals.begin(), vals.end(), std::back_inserter(stringvals),
                       [](T v) { return eckit::Translator<T, std::string>()(v); });
        std::string keyword = mapitr->second;

        LOG_DEBUG_LIB(LibMetkit) << "keyword " << keyword << " stringvals " << stringvals
                                 << std::endl;

        Type* t = language_.type(StringTools::lower(keyword));

        LOG_DEBUG_LIB(LibMetkit) << "---> stringvals: " << stringvals << std::endl;
        LOG_DEBUG_LIB(LibMetkit) << "     tidy: " << t->tidy(stringvals) << std::endl;

        request_.setValuesTyped(t, t->tidy(stringvals));
    }

    void operator()(const std::string& columnName, const std::set<long>& vals) override {
        LOG_DEBUG_LIB(LibMetkit) << "MarsRequestSetter::operator() columnName: " << columnName
                                 << " vals: " << vals << std::endl;

        auto mapitr = mapping_.find(columnName);
        ASSERT(mapitr != mapping_.end());

        std::set<std::string> mapped;
        if (IdMapper::instance().alphanumeric(mapitr->second, vals, mapped)) {
            visit(columnName, mapped);
        }
        else {
            visit(columnName, vals);
        }
    }

    void operator()(const std::string& columnName, const std::set<double>& vals) override {
        visit(columnName, vals);
    }
    void operator()(const std::string& columnName, const std::set<std::string>& vals) override {
        visit(columnName, vals);
    }

private:  // members
    MarsRequest& request_;
    MarsLanguage& language_;

    const StringDict& mapping_;
};

//----------------------------------------------------------------------------------------------------------------------

OdbToRequest::OdbToRequest(const std::string& verb, bool one, bool constant) :
    verb_(verb), one_(one), onlyConstantColumns_(constant) {
    LOG_DEBUG_LIB(LibMetkit) << "OdbToRequest one: " << one << " constant: " << constant
                             << std::endl;

    static PathName configPath(eckit::Resource<PathName>(
        "odbMarsRequestMapping", "~metkit/share/metkit/odb/marsrequest.yaml"));
    YAMLConfiguration config(configPath);

    for (const auto& key : config.keys()) {
        eckit::Log::debug<LibMetkit>()
            << "Mapping " << key << " -> " << config.getString(key) << std::endl;
        mapping_[config.getString(key)] = key;
    }
}


OdbToRequest::~OdbToRequest() {}


std::vector<MarsRequest> OdbToRequest::odbToRequest(DataHandle& dh) const {
    LOG_DEBUG_LIB(LibMetkit) << "OdbToRequest::odbToRequest() dh: " << dh << std::endl;

    Reader o(dh);
    Frame f(o);

    std::vector<MarsRequest> requests;

    std::vector<std::string> columnNames;
    columnNames.reserve(mapping_.size());
    for (const auto& kv : mapping_) {
        columnNames.push_back(kv.first);
    }

    MarsLanguage language(verb_);

    while (f.next(false)) {
        Span span = f.span(columnNames, onlyConstantColumns_);

        MarsRequest r(verb_);
        MarsRequestSetter setter(r, language, mapping_);
        span.visit(setter);

        if (one_ and requests.size()) {
            requests.back().merge(r);
        }
        else {
            requests.push_back(r);
        }
    }
    return requests;
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace odb
}  // namespace metkit
