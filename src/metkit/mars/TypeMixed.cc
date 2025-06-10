/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "metkit/mars/TypeMixed.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit {
namespace mars {

//----------------------------------------------------------------------------------------------------------------------

TypeMixed::TypeMixed(const std::string& name, const eckit::Value& settings) : Type(name, settings) {
    eckit::Value types = settings["type"];

    eckit::Value cfg;

    for (size_t i = 0; i < types.size(); ++i) {
        if (types[i].isString()) {
            cfg         = settings;
            cfg["type"] = types[i];

            Type* k = TypesFactory::build(name + "." + std::string(types[i]), cfg);
            k->attach();
            types_.emplace_back(nullptr, k);
        }
        else {  // it is a subtype, potentially with a Context
            cfg               = types[i];
            eckit::Value type = cfg["type"];

            std::unique_ptr<Context> c;
            if (cfg.contains("context")) {
                c = Context::parseContext(cfg["context"]);
            }

            Type* k = TypesFactory::build(name + "." + std::to_string(i) + "." + std::string(type), cfg);
            k->attach();
            types_.emplace_back(std::move(c), k);
        }
    }
}

TypeMixed::~TypeMixed() noexcept {
    for (auto it = types_.begin(); it != types_.end(); it++) {
        (*it).second->detach();
    }
}

void TypeMixed::print(std::ostream& out) const {
    out << "TypeMixed[name=" << name_;
    for (auto it = types_.begin(); it != types_.end(); it++) {
        out << "," << *((*it).second);
    }
    out << "]";
}


bool TypeMixed::expand(const MarsExpandContext& ctx, const MarsRequest& request, std::string& value) const {

    for (auto it = types_.begin(); it != types_.end(); it++) {
        if ((*it).first == nullptr || (*it).first->matches(request)) {
            std::string tmp = value;
            if ((*it).second->expand(ctx, request, tmp)) {
                value = tmp;
                return true;
            }
        }
    }
    return false;
}


static TypeBuilder<TypeMixed> type("mixed");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace mars
}  // namespace metkit
