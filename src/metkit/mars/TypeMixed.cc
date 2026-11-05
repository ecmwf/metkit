// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "metkit/mars/TypeMixed.h"
#include "metkit/mars/MarsRequest.h"
#include "metkit/mars/TypesFactory.h"


namespace metkit::mars {

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

bool TypeMixed::expand(std::string& value, const MarsRequest& request) const {

    for (auto it = types_.begin(); it != types_.end(); it++) {
        if ((*it).first == nullptr || (*it).first->matches(request)) {
            std::string tmp = value;
            if ((*it).second->expand(tmp, request)) {
                value = tmp;
                return true;
            }
        }
    }
    return false;
}


static TypeBuilder<TypeMixed> type("mixed");

//----------------------------------------------------------------------------------------------------------------------

}  // namespace metkit::mars
